"""Retarget the GASP crouch idle and crouch walk (UEFN skeleton) onto this project's UE5 Manny, into /Game/ThunderTemple/Anims."""
import unreal

LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/retarget_crouch.log"
SRC = ["/Game/Characters/UEFN_Mannequin/Animations/Idle/M_Neutral_Crouch_Idle_Loop",
       "/Game/Characters/UEFN_Mannequin/Animations/Crouch/M_Neutral_Crouch_Loop_F"]
RTG = "/Game/Characters/UE5_Mannequins/Rigs/RTG_UEFN_to_UE5_Mannequin"
OUT = "/Game/ThunderTemple/Anims"

lines = []
try:
    eal = unreal.EditorAssetLibrary
    bp = eal.load_blueprint_class("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter")
    cdo = unreal.get_default_object(bp)
    target_mesh = cdo.get_editor_property("mesh").get_editor_property("skeletal_mesh_asset")
    lines.append("target mesh: %s" % target_mesh.get_path_name())

    anims = [eal.load_asset(p) for p in SRC]
    src_skel = anims[0].get_editor_property("skeleton")
    # any mesh on the source skeleton works as the retarget source
    src_mesh = None
    for p in eal.list_assets("/Game/Characters/UEFN_Mannequin/Meshes", recursive=True):
        a = eal.load_asset(p)
        if isinstance(a, unreal.SkeletalMesh) and a.get_editor_property("skeleton") == src_skel:
            src_mesh = a
            break
    lines.append("source mesh: %s" % (src_mesh.get_path_name() if src_mesh else None))

    rtg = eal.load_asset(RTG)
    data = [unreal.AssetData(object_path=a.get_path_name()) if hasattr(unreal.AssetData, "object_path") else unreal.EditorAssetLibrary.find_asset_data(a.get_path_name()) for a in anims]
    data = [eal.find_asset_data(a.get_path_name()) for a in anims]
    lines.append((unreal.IKRetargetBatchOperation.duplicate_and_retarget.__doc__ or "")[:300])
    eal.make_directory(OUT)
    new = unreal.IKRetargetBatchOperation.duplicate_and_retarget(data, src_mesh, target_mesh, rtg, "", "", "", "_Manny", OUT, False)
    for d in new:
        path = str(d.package_name)
        dest = path
        a = eal.load_asset(dest)
        eal.save_loaded_asset(a)
        lines.append("created %s (skeleton %s, %.2fs)" % (dest, a.get_editor_property("skeleton").get_path_name(), a.get_play_length()))
except Exception as e:
    import traceback
    lines.append("ERROR %r\n%s" % (e, traceback.format_exc()))
with open(LOG, "w") as f:
    f.write("\n".join(lines))
