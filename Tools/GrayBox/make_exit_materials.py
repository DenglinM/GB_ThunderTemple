"""MI_GB_BlueDark (exit locked) and MI_GB_BlueBright (exit open), made from MI_GB_Blue."""
import unreal

LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/make_exit_materials.log"
lines = []
try:
    eal = unreal.EditorAssetLibrary
    mel = unreal.MaterialEditingLibrary
    base = eal.load_asset("/Game/ThunderTemple/Materials/MI_GB_Blue")
    names = mel.get_vector_parameter_names(base)
    lines.append("params %s, value %s" % (names, [mel.get_material_instance_vector_parameter_value(base, n) for n in names]))
    for name, color in [("MI_GB_BlueDark", unreal.LinearColor(0.01, 0.02, 0.12, 1.0)), ("MI_GB_BlueBright", unreal.LinearColor(0.25, 0.75, 1.0, 1.0))]:
        path = "/Game/ThunderTemple/Materials/" + name
        if not eal.does_asset_exist(path):
            eal.duplicate_asset("/Game/ThunderTemple/Materials/MI_GB_Blue", path)
        mi = eal.load_asset(path)
        for n in names:
            mel.set_material_instance_vector_parameter_value(mi, n, color)
        lines.append("%s saved %s" % (name, eal.save_loaded_asset(mi, False)))
except Exception as e:
    import traceback
    lines.append("ERROR %r\n%s" % (e, traceback.format_exc()))
with open(LOG, "w") as f:
    f.write("\n".join(lines))
