"""M_GB_Glow: base color plus emissive, either over the whole surface (Fill 1) or only along each face's edges (Fill 0).
Glow instances copy the surface color of the matching gray-box material."""
import unreal

LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/make_glow_materials.log"
DIR = "/Game/ThunderTemple/Materials"
# name, source instance, emissive strength, fill (1 whole surface, 0 edges only), edge width in UV
GLOWS = [("MI_GB_Yellow_Glow", "MI_GB_Yellow", 1.5, 1.0, 0.06),
         ("MI_GB_Enemy_Glow", "MI_GB_Enemy", 1.0, 1.0, 0.06),
         ("MI_GB_Blue_EdgeGlow", "MI_GB_Blue", 6.0, 0.0, 0.05),
         ("MI_GB_BlueBright_Glow", "MI_GB_BlueBright", 0.6, 1.0, 0.06)]
lines = []
try:
    eal = unreal.EditorAssetLibrary
    mel = unreal.MaterialEditingLibrary
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    path = DIR + "/M_GB_Glow"
    if eal.does_asset_exist(path):
        m = eal.load_asset(path)
        mel.delete_all_material_expressions(m)
    else:
        m = tools.create_asset("M_GB_Glow", DIR, unreal.Material, unreal.MaterialFactoryNew())
    E = lambda cls, x, y: mel.create_material_expression(m, cls, x, y)
    color = E(unreal.MaterialExpressionVectorParameter, -1400, 0)
    color.set_editor_property("parameter_name", "Color")
    color.set_editor_property("default_value", unreal.LinearColor(0.5, 0.5, 0.5, 1))
    strength = E(unreal.MaterialExpressionScalarParameter, -1400, 200)
    strength.set_editor_property("parameter_name", "EmissiveStrength")
    strength.set_editor_property("default_value", 1.0)
    fill = E(unreal.MaterialExpressionScalarParameter, -1400, 300)
    fill.set_editor_property("parameter_name", "Fill")
    fill.set_editor_property("default_value", 1.0)
    width = E(unreal.MaterialExpressionScalarParameter, -1400, 400)
    width.set_editor_property("parameter_name", "EdgeWidth")
    width.set_editor_property("default_value", 0.05)
    # distance to the nearest UV border of the face
    uv = E(unreal.MaterialExpressionTextureCoordinate, -1400, 550)
    one_minus = E(unreal.MaterialExpressionOneMinus, -1200, 650)
    mel.connect_material_expressions(uv, "", one_minus, "")
    near = E(unreal.MaterialExpressionMin, -1050, 550)
    mel.connect_material_expressions(uv, "", near, "A")
    mel.connect_material_expressions(one_minus, "", near, "B")
    mr = E(unreal.MaterialExpressionComponentMask, -900, 500)
    mr.set_editor_property("r", True)
    mg = E(unreal.MaterialExpressionComponentMask, -900, 600)
    mg.set_editor_property("g", True)
    mel.connect_material_expressions(near, "", mr, "")
    mel.connect_material_expressions(near, "", mg, "")
    d = E(unreal.MaterialExpressionMin, -750, 550)
    mel.connect_material_expressions(mr, "", d, "A")
    mel.connect_material_expressions(mg, "", d, "B")
    # 1 inside the edge band, 0 elsewhere
    sub = E(unreal.MaterialExpressionSubtract, -600, 450)
    mel.connect_material_expressions(width, "", sub, "A")
    mel.connect_material_expressions(d, "", sub, "B")
    sharp = E(unreal.MaterialExpressionMultiply, -450, 450)
    sharp.set_editor_property("const_b", 1000.0)
    mel.connect_material_expressions(sub, "", sharp, "A")
    edge = E(unreal.MaterialExpressionSaturate, -300, 450)
    mel.connect_material_expressions(sharp, "", edge, "")
    mask = E(unreal.MaterialExpressionLinearInterpolate, -150, 350)
    mel.connect_material_expressions(fill, "", mask, "A")
    mask.set_editor_property("const_b", 1.0)
    mel.connect_material_expressions(edge, "", mask, "Alpha")
    glow = E(unreal.MaterialExpressionMultiply, -150, 150)
    mel.connect_material_expressions(color, "", glow, "A")
    mel.connect_material_expressions(strength, "", glow, "B")
    emissive = E(unreal.MaterialExpressionMultiply, 0, 200)
    mel.connect_material_expressions(glow, "", emissive, "A")
    mel.connect_material_expressions(mask, "", emissive, "B")
    rough = E(unreal.MaterialExpressionConstant, -150, -100)
    rough.set_editor_property("r", 0.7)
    mel.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    mel.recompile_material(m)
    lines.append("master saved %s" % eal.save_loaded_asset(m, False))

    for name, src, s, f, w in GLOWS:
        base = eal.load_asset(DIR + "/" + src)
        c = mel.get_material_instance_vector_parameter_value(base, "SurfaceColor")
        p = DIR + "/" + name
        mi = eal.load_asset(p) if eal.does_asset_exist(p) else tools.create_asset(name, DIR, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        mel.set_material_instance_parent(mi, m)
        mel.set_material_instance_vector_parameter_value(mi, "Color", c)
        mel.set_material_instance_scalar_parameter_value(mi, "EmissiveStrength", s)
        mel.set_material_instance_scalar_parameter_value(mi, "Fill", f)
        mel.set_material_instance_scalar_parameter_value(mi, "EdgeWidth", w)
        lines.append("%s color %s saved %s" % (name, c, eal.save_loaded_asset(mi, False)))
except Exception as e:
    import traceback
    lines.append("ERROR %r\n%s" % (e, traceback.format_exc()))
with open(LOG, "w") as f:
    f.write("\n".join(lines))
