"""Import Level_base.udatasmith into the currently open level; log to Saved/import_rhino_datasmith.log."""
import unreal

SRC = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Level_base.udatasmith"
DEST = "/Game/ThunderTemple/Rhino"
LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/import_rhino_datasmith.log"

lines = []
try:
    scene = unreal.DatasmithSceneElement.construct_datasmith_scene_from_file(SRC)
    if scene is None:
        raise RuntimeError("could not read " + SRC)
    opts = scene.get_options(unreal.DatasmithImportOptions)
    opts.base_options.scene_handling = unreal.DatasmithImportScene.CURRENT_LEVEL
    result = scene.import_scene(DEST)
    lines.append("import_succeed=%s" % result.import_succeed)
    meshes = list(result.imported_meshes)
    lines.append("meshes=%d actors=%d" % (len(meshes), len(result.imported_actors)))
    # Rhino surfaces have no simple collision; walk on the render mesh.
    for sm in meshes:
        bs = sm.get_editor_property("body_setup")
        if bs:
            bs.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        unreal.EditorAssetLibrary.save_loaded_asset(sm)
    for a in result.imported_actors:
        b = a.get_actor_bounds(False)
        lines.append("actor %s origin=%s extent=%s" % (a.get_actor_label(), b[0], b[1]))
    scene.destroy_scene()
except Exception as e:
    lines.append("ERROR %r" % e)

with open(LOG, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))
