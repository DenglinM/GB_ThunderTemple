"""Dump world-space triangles (positions inline) of every imported Rhino mesh actor to Saved/rhino_tris.json."""
import json
import unreal

OUT = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/rhino_tris.json"
LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/export_tris.log"

lines = []
data = {}
try:
    for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        lab = a.get_actor_label()
        if not isinstance(a, unreal.StaticMeshActor) or not (lab.startswith("mesh_") or lab.startswith("brep")):
            continue
        comp = a.static_mesh_component
        xf = comp.get_world_transform()
        md = comp.static_mesh.get_static_mesh_description(0)
        tris = []
        for ti in range(md.get_triangle_count()):
            # the binding returns 6 entries here; only the last three are real vertex ids
            vids = list(md.get_triangle_vertices(unreal.TriangleID(ti)))[-3:]
            if ti == 0:
                lines.append("%s first tri raw=%r" % (lab, vids))
            tri = []
            for v in vids:
                p = xf.transform_location(md.get_vertex_position(v))
                tri.append([round(p.x, 1), round(p.y, 1), round(p.z, 1)])
            tris.append(tri)
        data[lab] = tris
        lines.append("%s tris=%d" % (lab, len(tris)))
    with open(OUT, "w") as f:
        json.dump(data, f)
except Exception as e:
    import traceback
    lines.append("ERROR %r\n%s" % (e, traceback.format_exc()))

with open(LOG, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))
