"""Clean re-import of the Rhino base into Lvl_ThunderTemple_Rhino.

Removes every Datasmith actor left from earlier imports, imports Level_base.udatasmith into a fresh folder,
scales it to 50%, drops the pieces that were rebuilt natively (bridges mesh_7/8/9, pyramids brep/brep_2)
and the Rhino view cameras, applies the gray-box colours, then saves the level so deleted actors'
external package files are removed from disk too.
"""
import re
import unreal

SRC = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Level_base.udatasmith"
OLD_DIR = "/Game/ThunderTemple/Rhino"
DEST = "/Game/ThunderTemple/RhinoBase"
SCALE = 0.5
REBUILT = {"mesh_7", "mesh_8", "mesh_9", "brep", "brep_2"}
FLOOR_EXTRA = {"brep_9"}
WALL_MIN_HEIGHT = 250.0      # cm, after scaling
LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/reimport_rhino_clean.log"
DATASMITH_LABEL = re.compile(r"(Level_base|mesh|temple|mesh_\d+|brep(_\d+)?|Current_\w+)")

lines = []
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
eal = unreal.EditorAssetLibrary


def save_level():
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()


try:
    # 1. remove every Datasmith actor from earlier imports
    old = [a for a in actors.get_all_level_actors() if DATASMITH_LABEL.fullmatch(a.get_actor_label())]
    for a in old:
        actors.destroy_actor(a)
    lines.append("removed %d old actors" % len(old))
    save_level()

    # 2. drop the old imported assets (nothing references them any more)
    if eal.does_directory_exist(OLD_DIR):
        lines.append("deleted %s: %s" % (OLD_DIR, eal.delete_directory(OLD_DIR)))

    # 3. fresh import
    scene = unreal.DatasmithSceneElement.construct_datasmith_scene_from_file(SRC)
    opts = scene.get_options(unreal.DatasmithImportOptions)
    opts.base_options.scene_handling = unreal.DatasmithImportScene.CURRENT_LEVEL
    result = scene.import_scene(DEST)
    scene.destroy_scene()
    lines.append("import_succeed=%s meshes=%d actors=%d" % (result.import_succeed, len(result.imported_meshes), len(result.imported_actors)))
    for sm in result.imported_meshes:
        bs = sm.get_editor_property("body_setup")
        if bs:
            bs.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        eal.save_loaded_asset(sm)

    # 4. scale, prune, colour
    floor_mi = eal.load_asset("/Game/ThunderTemple/Materials/MI_GB_Floor")
    wall_mi = eal.load_asset("/Game/ThunderTemple/Materials/MI_GB_Wall")
    new = list(result.imported_actors)
    for a in new:
        if a.get_actor_label() == "Level_base":
            a.set_actor_scale3d(unreal.Vector(SCALE, SCALE, SCALE))
    kept = {"floor": [], "wall": []}
    for a in new:
        lab = a.get_actor_label()
        if lab in REBUILT or lab.startswith("Current_"):
            actors.destroy_actor(a)
            continue
        if not isinstance(a, unreal.StaticMeshActor):
            continue
        origin, extent = a.get_actor_bounds(False)
        kind = "floor" if (extent.z * 2 < WALL_MIN_HEIGHT or lab in FLOOR_EXTRA) else "wall"
        a.static_mesh_component.set_editor_property("override_materials", [floor_mi if kind == "floor" else wall_mi])
        kept[kind].append(lab)
    lines.append("floor: " + ", ".join(sorted(kept["floor"])))
    lines.append("wall: " + ", ".join(sorted(kept["wall"])))
    save_level()
    lines.append("saved")
except Exception as e:
    import traceback
    lines.append("ERROR %r\n%s" % (e, traceback.format_exc()))

with open(LOG, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))
