"""Reverse polygon facing on the two pyramids in the core space (Rhino export had inverted normals)."""
import unreal

GEO = "/Game/ThunderTemple/Rhino/Level_base/Geometries/"
TARGETS = ["brep", "brep_2"]
LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/flip_normals.log"

lines = []
try:
    for name in TARGETS:
        sm = unreal.EditorAssetLibrary.load_asset(GEO + name)
        md = sm.get_static_mesh_description(0)
        n = md.get_polygon_count()
        for i in range(n):
            md.reverse_polygon_facing(unreal.PolygonID(i))
        sm.build_from_static_mesh_descriptions([md])
        unreal.EditorAssetLibrary.save_loaded_asset(sm)
        lines.append("%s flipped %d polygons" % (name, n))
except Exception as e:
    lines.append("ERROR %r" % e)

with open(LOG, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))
