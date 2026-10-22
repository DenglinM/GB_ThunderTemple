"""Build closed pyramid meshes matching the two broken Rhino pyramids (brep, brep_2) and place them; remove the originals.
Coordinates are world cm at the current 0.5 Rhino scale; base z dropped to the core floor so they sit on it."""
import unreal

LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/make_pyramids.log"
FLOOR_Z = 95.0
PYRAMIDS = {
    "SM_GB_PyramidA": {"replaces": "brep", "apex": (2765.9, -809.5, 339.4),
                       "base": [(2544.4, -1185.2), (3209.9, -810.8), (2987.4, -433.8), (2480.1, -880.1)]},
    "SM_GB_PyramidB": {"replaces": "brep_2", "apex": (3762.8, -1897.5, 340.8),
                       "base": [(3505.5, -1795.2), (3683.7, -1544.5), (3960.0, -1873.3), (3755.3, -2174.3)]},
}
DIR = "/Game/ThunderTemple/Meshes"
MAT = "/Game/ThunderTemple/Materials/MI_GB_Wall"

lines = []
try:
    eal = unreal.EditorAssetLibrary
    mat = eal.load_asset(MAT)
    actor_sys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    for name, p in PYRAMIDS.items():
        cx = sum(b[0] for b in p["base"]) / 4
        cy = sum(b[1] for b in p["base"]) / 4
        path = DIR + "/" + name
        if not eal.does_asset_exist(path):
            eal.duplicate_asset("/Engine/BasicShapes/Cube", path)
        sm = eal.load_asset(path)
        md = sm.create_static_mesh_description()
        pg = md.create_polygon_group()
        local = [unreal.Vector(b[0] - cx, b[1] - cy, 0.0) for b in p["base"]]
        apex = unreal.Vector(p["apex"][0] - cx, p["apex"][1] - cy, p["apex"][2] - FLOOR_Z)
        vids = []
        for v in local + [apex]:
            vid = md.create_vertex()
            md.set_vertex_position(vid, v)
            vids.append(vid)

        def face(idx):
            for tri in ([idx[0], idx[1], idx[2]],) if len(idx) == 3 else ([idx[0], idx[1], idx[2]], [idx[0], idx[2], idx[3]]):
                md.create_triangle(pg, [md.create_vertex_instance(vids[i]) for i in tri])

        # base corners are counter-clockwise seen from above; sides face outward, base faces down
        ccw = (local[1].x - local[0].x) * (local[2].y - local[0].y) - (local[2].x - local[0].x) * (local[1].y - local[0].y) > 0
        order = [0, 1, 2, 3] if ccw else [3, 2, 1, 0]
        for k in range(4):
            a, b = order[k], order[(k + 1) % 4]
            face([a, 4, b])          # UE is left-handed: this winding faces outward
        face([order[0], order[1], order[2], order[3]])
        sm.build_from_static_mesh_descriptions([md])
        sm.set_material(0, mat)
        bs = sm.get_editor_property("body_setup")
        bs.set_editor_property("collision_trace_flag", unreal.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
        eal.save_loaded_asset(sm)

        for a in actor_sys.get_all_level_actors():
            if a.get_actor_label() == p["replaces"]:
                actor_sys.destroy_actor(a)
        act = actor_sys.spawn_actor_from_object(sm, unreal.Vector(cx, cy, FLOOR_Z))
        act.set_actor_label("GB_" + name[6:])
        act.set_folder_path("GrayBox/Rebuilt")
        lines.append("%s built, placed at (%.0f, %.0f, %.0f), tris=%d" % (name, cx, cy, FLOOR_Z, md.get_triangle_count()))
except Exception as e:
    import traceback
    lines.append("ERROR %r\n%s" % (e, traceback.format_exc()))

with open(LOG, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))
