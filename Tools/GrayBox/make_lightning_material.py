"""Unlit, very bright material for the lightning bolts."""
import unreal

PATH, NAME = "/Game/ThunderTemple/Materials", "M_GB_Lightning"
LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/make_lightning_material.log"

try:
    eal = unreal.EditorAssetLibrary
    mel = unreal.MaterialEditingLibrary
    if eal.does_asset_exist(PATH + "/" + NAME):
        mat = eal.load_asset(PATH + "/" + NAME)
    else:
        mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME, PATH, unreal.Material, unreal.MaterialFactoryNew())
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    col = mel.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -300, 0)
    col.set_editor_property("constant", unreal.LinearColor(12.0, 16.0, 30.0, 1.0))
    mel.connect_material_property(col, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.recompile_material(mat)
    eal.save_loaded_asset(mat)
    msg = "ok"
except Exception as e:
    msg = "ERROR %r" % e

with open(LOG, "w") as f:
    f.write(msg)
