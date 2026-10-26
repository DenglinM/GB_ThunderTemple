"""Renders LS_LevelFlythrough with Movie Render Queue to Render/: 1080p MP4 plus a WAV of the audio.
MRQ renders off-screen, so the result does not depend on the editor window size or Windows display scaling."""
import unreal

LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/render_flythrough.log"
OUT = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Render"
lines = []
try:
    subsystem = unreal.get_editor_subsystem(unreal.MoviePipelineQueueSubsystem)
    queue = subsystem.get_queue()
    queue.delete_all_jobs()
    job = queue.allocate_new_job(unreal.MoviePipelineExecutorJob)
    job.job_name = "LevelFlythrough"
    job.sequence = unreal.SoftObjectPath("/Game/ThunderTemple/Cinematics/LS_LevelFlythrough.LS_LevelFlythrough")
    job.map = unreal.SoftObjectPath("/Game/ThunderTemple/Lvl_ThunderTemple_Rhino.Lvl_ThunderTemple_Rhino")
    config = job.get_configuration()
    config.find_or_add_setting_by_class(unreal.MoviePipelineDeferredPassBase)
    out = config.find_or_add_setting_by_class(unreal.MoviePipelineOutputSetting)
    out.output_directory = unreal.DirectoryPath(OUT)
    out.output_resolution = unreal.IntPoint(1920, 1080)
    out.file_name_format = "{sequence_name}"
    out.override_existing_output = True
    # let Lumen and auto effects settle before the first frame is kept
    aa = config.find_or_add_setting_by_class(unreal.MoviePipelineAntiAliasingSetting)
    aa.engine_warm_up_count = 60
    aa.render_warm_up_frames = True
    config.find_or_add_setting_by_class(unreal.MoviePipelineMP4EncoderOutput)
    config.find_or_add_setting_by_class(unreal.MoviePipelineWaveOutput)
    executor = subsystem.render_queue_with_executor(unreal.MoviePipelinePIEExecutor)
    lines.append("render started -> %s" % OUT)
except Exception as e:
    import traceback
    lines.append("ERROR %r\n%s" % (e, traceback.format_exc()))
with open(LOG, "w") as f:
    f.write("\n".join(lines))
