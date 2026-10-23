"""Make the retargeted crouch anims self-contained: drop GASP footstep notifies and use the engine curve compression."""
import unreal

LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/clean_crouch_anims.log"
lines = []
try:
    eal = unreal.EditorAssetLibrary
    curve = eal.load_asset("/Engine/Animation/DefaultAnimCurveCompressionSettings")
    for p in ["/Game/ThunderTemple/Anims/M_Neutral_Crouch_Idle_Loop_Manny", "/Game/ThunderTemple/Anims/M_Neutral_Crouch_Loop_F_Manny"]:
        a = eal.load_asset(p)
        unreal.AnimationLibrary.remove_all_animation_notify_tracks(a)
        a.set_editor_property("curve_compression_settings", curve)
        a.modify()
        eal.save_loaded_asset(a, False)
        lines.append("%s deps: %s" % (p.split("/")[-1], [str(d) for d in unreal.AssetRegistryHelpers.get_asset_registry().get_dependencies(p, unreal.AssetRegistryDependencyOptions())]))
except Exception as e:
    import traceback
    lines.append("ERROR %r\n%s" % (e, traceback.format_exc()))
with open(LOG, "w") as f:
    f.write("\n".join(lines))
