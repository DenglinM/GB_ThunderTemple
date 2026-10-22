"""Rhino ramps exported as single surfaces: make their collision double-sided and give them a two-sided floor material."""
import unreal

GEO = "/Game/ThunderTemple/Rhino/Level_base/Geometries/"
SURFACES = ["mesh_7", "mesh_8", "mesh_9"]
SRC_MI = "/Game/ThunderTemple/Materials/MI_GB_Floor"
DST_MI = "/Game/ThunderTemple/Materials/MI_GB_Floor_TwoSided"
LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/fix_surface_meshes.log"

lines = []
try:
    eal = unreal.EditorAssetLibrary
    if not eal.does_asset_exist(DST_MI):
        eal.duplicate_asset(SRC_MI, DST_MI)
    mi = eal.load_asset(DST_MI)
    ov = mi.get_editor_property("base_property_overrides")
    ov.set_editor_property("override_two_sided", True)
    ov.set_editor_property("two_sided", True)
    mi.set_editor_property("base_property_overrides", ov)
    unreal.MaterialEditingLibrary.update_material_instance(mi)
    eal.save_loaded_asset(mi)

    for name in SURFACES:
        sm = eal.load_asset(GEO + name)
        bs = sm.get_editor_property("body_setup")
        bs.set_editor_property("double_sided_geometry", True)
        bs.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        sm.set_material(0, mi)
        eal.save_loaded_asset(sm)
        lines.append("%s double_sided=%s" % (name, bs.get_editor_property("double_sided_geometry")))

    for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        if a.get_actor_label() in SURFACES:
            for c in a.get_components_by_class(unreal.StaticMeshComponent):
                c.set_editor_property("override_materials", [mi])
                c.recreate_physics_state()
            lines.append("actor %s updated" % a.get_actor_label())
except Exception as e:
    lines.append("ERROR %r" % e)

with open(LOG, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))
