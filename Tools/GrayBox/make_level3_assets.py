"""Level 3 assets: IA_Interact bound to E in IMC_Default, and the drum beat sound."""
import unreal

LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/l3_assets.log"
lines = []
try:
    eal = unreal.EditorAssetLibrary
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    ia_path = "/Game/Input/Actions/IA_Interact"
    if eal.does_asset_exist(ia_path):
        ia = eal.load_asset(ia_path)
    else:
        ia = tools.create_asset("IA_Interact", "/Game/Input/Actions", unreal.InputAction, unreal.InputAction_Factory())
    ia.set_editor_property("value_type", unreal.InputActionValueType.BOOLEAN)
    eal.save_loaded_asset(ia, False)
    imc = eal.load_asset("/Game/Input/IMC_Default")
    if not any(m.action == ia for m in imc.get_editor_property("mappings")):
        k = unreal.Key()
        k.import_text("E")
        imc.map_key(ia, k)
    imc.modify()
    lines.append("imc saved %s, mappings %s" % (eal.save_loaded_asset(imc, False), [(m.action.get_name() if m.action else None, m.key.export_text()) for m in imc.get_editor_property("mappings")]))
    task = unreal.AssetImportTask()
    task.filename = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/import/S_Drum_Beat.wav"
    task.destination_path = "/Game/ThunderTemple/SE_Drum"
    task.automated = True
    task.replace_existing = True
    task.save = True
    tools.import_asset_tasks([task])
    lines.append("imported %s" % list(task.imported_object_paths))
except Exception as e:
    import traceback
    lines.append("ERROR %r\n%s" % (e, traceback.format_exc()))
with open(LOG, "w") as f:
    f.write("\n".join(lines))
