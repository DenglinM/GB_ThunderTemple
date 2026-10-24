"""M_GB_SoundWave: unlit, translucent, two-sided cyan shell, brighter at the rim, for the finale thunder wave."""
import unreal

LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/make_wave_material.log"
lines = []
try:
    eal = unreal.EditorAssetLibrary
    mel = unreal.MaterialEditingLibrary
    path = "/Game/ThunderTemple/Materials/M_GB_SoundWave"
    if eal.does_asset_exist(path):
        eal.delete_asset(path)
    m = unreal.AssetToolsHelpers.get_asset_tools().create_asset("M_GB_SoundWave", "/Game/ThunderTemple/Materials", unreal.Material, unreal.MaterialFactoryNew())
    m.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    m.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    m.set_editor_property("two_sided", True)
    color = mel.create_material_expression(m, unreal.MaterialExpressionConstant3Vector, -500, 0)
    color.set_editor_property("constant", unreal.LinearColor(0.35, 0.85, 1.0, 1.0))
    fres = mel.create_material_expression(m, unreal.MaterialExpressionFresnel, -700, 200)
    fres.set_editor_property("exponent", 2.5)
    lerp = mel.create_material_expression(m, unreal.MaterialExpressionLinearInterpolate, -300, 200)
    lerp.set_editor_property("const_a", 0.08)
    lerp.set_editor_property("const_b", 0.6)
    mel.connect_material_expressions(fres, "", lerp, "Alpha")
    glow = mel.create_material_expression(m, unreal.MaterialExpressionMultiply, -300, 0)
    glow.set_editor_property("const_b", 3.0)
    mel.connect_material_expressions(color, "", glow, "A")
    mel.connect_material_property(glow, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(lerp, "", unreal.MaterialProperty.MP_OPACITY)
    mel.recompile_material(m)
    lines.append("saved %s" % eal.save_loaded_asset(m, False))
except Exception as e:
    import traceback
    lines.append("ERROR %r\n%s" % (e, traceback.format_exc()))
with open(LOG, "w") as f:
    f.write("\n".join(lines))
