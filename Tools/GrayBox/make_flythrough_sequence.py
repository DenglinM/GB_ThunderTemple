"""Builds LS_LevelFlythrough: a camera tour from the player start over the core, a full orbit around Levels 1-3
with several lightning strikes, and back to the start.

Lightning is staged inside the sequence, not by the gameplay rods: each strike is a spawnable bolt (a stretched
cylinder with the lightning material) and a spawnable point light, both spawned only for a short double flicker,
plus the thunder sound after the same distance-based delay the game uses. Run inside the editor with the
Thunder Temple level open.
"""
import math
import unreal

LOG = "D:/Game/miHoYo_LevelDesign/GB_ThunderTemple/Saved/make_flythrough_sequence.log"
SEQ_DIR = "/Game/ThunderTemple/Cinematics"
SEQ_NAME = "LS_LevelFlythrough"
FPS = 30
BOLT_MATERIAL = "/Game/ThunderTemple/Materials/M_GB_Lightning"
THUNDER = ["/Game/ThunderTemple/SE_Thunder/universfield-loud-thunder-192165",
           "/Game/ThunderTemple/SE_Thunder/u_vrs223ln83-loud-thunder-439064",
           "/Game/ThunderTemple/SE_Thunder/universfield-thunder-strike-124463"]
# (time in seconds, rod label)
STRIKES = [(12.5, "GB_RodSub_CoreA"), (13.0, "GB_RodSub_CoreB"), (20.5, "GB_RodSub_L1_A"),
           (26.0, "GB_RodSub_L2_B"), (31.0, "GB_RodSub_L1_B"), (35.5, "GB_RodSub_Tutorial1")]

lines = []


def frame(seconds):
    return unreal.FrameNumber(int(round(seconds * FPS)))


def look_rotation(cam, target):
    dx, dy, dz = target[0] - cam[0], target[1] - cam[1], target[2] - cam[2]
    yaw = math.degrees(math.atan2(dy, dx))
    pitch = math.degrees(math.atan2(dz, math.hypot(dx, dy)))
    return pitch, yaw


def find_actor(label):
    for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
        if a.get_actor_label() == label:
            return a
    return None


def channels(section):
    return section.get_all_channels()


try:
    eal = unreal.EditorAssetLibrary
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    start_actor = find_actor("PlayerStart") or [a for a in actors.get_all_level_actors() if isinstance(a, unreal.PlayerStart)][0]
    main = find_actor("GB_RodMain")
    s = start_actor.get_actor_location()
    start_yaw = math.radians(start_actor.get_actor_rotation().yaw)
    fwd = (math.cos(start_yaw), math.sin(start_yaw))
    m = main.get_actor_location()

    # --- camera path: (time, camera location, look-at point) ---
    start_cam = (s.x - 450 * fwd[0], s.y - 450 * fwd[1], s.z + 260)
    start_look = (s.x + 2500 * fwd[0], s.y + 2500 * fwd[1], s.z - 100)
    keys = [(0.0, start_cam, start_look),
            (4.0, (700, 3300, 700), (1600, 1500, -200)),
            (8.0, (2100, 900, 1000), (3000, -900, 100)),
            (12.0, (2500, -500, 2300), (m.x, m.y, m.z + 600))]
    # full orbit around the whole temple, looking at its middle so Levels 1-3 pass in turn
    center, radius, height = (2500, -500, 700), 7200, 4600
    orbit_start, orbit_end, theta0 = 16.5, 35.0, -70.0
    steps = 18
    for i in range(steps + 1):
        a = math.radians(theta0 + 360.0 * i / steps)
        keys.append((orbit_start + (orbit_end - orbit_start) * i / steps,
                     (center[0] + radius * math.cos(a), center[1] + radius * math.sin(a), height), center))
    keys += [(38.5, (600, 6600, 1600), (s.x, s.y, s.z)),
             (42.0, start_cam, start_look)]
    end_time = 42.0

    # --- sequence asset ---
    path = SEQ_DIR + "/" + SEQ_NAME
    # rebuild in place: an asset deleted earlier this session can block re-creating the same name
    if eal.does_asset_exist(path):
        seq = eal.load_asset(path)
        for binding in seq.get_bindings():
            binding.remove()
        for track in seq.get_tracks():
            seq.remove_track(track)
    else:
        seq = unreal.AssetToolsHelpers.get_asset_tools().create_asset(SEQ_NAME, SEQ_DIR, unreal.LevelSequence, unreal.LevelSequenceFactoryNew())
    seq.set_display_rate(unreal.FrameRate(FPS, 1))
    seq.set_tick_resolution(unreal.FrameRate(FPS, 1))
    seq.set_playback_start(0)
    seq.set_playback_end(int(end_time * FPS))

    # --- camera ---
    cam = seq.add_spawnable_from_class(unreal.CineCameraActor)
    cam.set_display_name("FlythroughCamera")
    template = cam.get_object_template()
    # wide lens and no depth of field, so the whole temple stays sharp
    cine = template.get_cine_camera_component() or template.get_editor_property("camera_component")
    if cine:
        focus = cine.get_editor_property("focus_settings")
        focus.set_editor_property("focus_method", unreal.CameraFocusMethod.DISABLE)
        cine.set_editor_property("focus_settings", focus)
        cine.set_editor_property("current_focal_length", 18.0)
    else:
        lines.append("camera template has no cine component; default lens kept")

    xf = cam.add_track(unreal.MovieScene3DTransformTrack).add_section()
    xf.set_range(0, int(end_time * FPS))
    ch = channels(xf)  # location x y z, rotation roll pitch yaw, scale x y z
    prev_yaw = None
    for t, loc, look in keys:
        pitch, yaw = look_rotation(loc, look)
        # keep yaw continuous so the orbit never spins the wrong way round
        if prev_yaw is not None:
            while yaw - prev_yaw > 180:
                yaw -= 360
            while yaw - prev_yaw < -180:
                yaw += 360
        prev_yaw = yaw
        f = frame(t)
        for c, v in zip(ch[0:6], [loc[0], loc[1], loc[2], 0.0, pitch, yaw]):
            c.add_key(f, v, interpolation=unreal.MovieSceneKeyInterpolation.AUTO)

    cut = seq.add_track(unreal.MovieSceneCameraCutTrack).add_section()
    cut.set_range(0, int(end_time * FPS))
    cut.set_camera_binding_id(seq.get_binding_id(cam))

    # --- lightning strikes ---
    pole_main = main.get_editor_property("PoleHeight") + main.get_editor_property("PoleRadius") * 2.0
    origin = unreal.Vector(m.x, m.y, m.z + pole_main)
    linear = main.get_editor_property("bLinearDelay")
    bolt_mat = eal.load_asset(BOLT_MATERIAL)
    cyl = eal.load_asset("/Engine/BasicShapes/Cylinder")
    audio = seq.add_track(unreal.MovieSceneAudioTrack)
    for n, (t, label) in enumerate(STRIKES):
        rod = find_actor(label)
        if not rod:
            lines.append("missing rod %s" % label)
            continue
        r = rod.get_actor_location()
        tip = unreal.Vector(r.x, r.y, r.z + rod.get_editor_property("PoleHeight"))
        d = tip - origin
        length = d.length()
        mid = origin + d * 0.5
        # the cylinder runs along its local Z; a negative pitch of the bolt's angle from vertical, at the bolt's yaw, tips Z onto it
        pitch = -math.degrees(math.atan2(math.hypot(d.x, d.y), d.z))
        yaw = math.degrees(math.atan2(d.y, d.x))
        if linear:
            delay = length / main.get_editor_property("LinearDistancePerSecond")
        else:
            ground = math.hypot(r.x - m.x, r.y - m.y)
            delay = max(1.0, math.ceil(ground / main.get_editor_property("RingSpacing"))) * main.get_editor_property("SecondsPerRing")

        bolt = seq.add_spawnable_from_class(unreal.StaticMeshActor)
        bolt.set_display_name("Bolt_%s" % label)
        bt = bolt.get_object_template()
        smc = bt.get_editor_property("static_mesh_component")
        smc.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
        smc.set_static_mesh(cyl)
        smc.set_material(0, bolt_mat)
        smc.set_editor_property("cast_shadow", False)
        smc.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
        bxf = bolt.add_track(unreal.MovieScene3DTransformTrack).add_section()
        bxf.set_range(0, int(end_time * FPS))
        bch = channels(bxf)
        for c, v in zip(bch, [mid.x, mid.y, mid.z, 0.0, pitch, yaw, 0.3, 0.3, length / 100.0]):
            c.add_key(frame(0), v)

        light = seq.add_spawnable_from_class(unreal.PointLight)
        light.set_display_name("Flash_%s" % label)
        lc = light.get_object_template().get_editor_property("point_light_component")
        lc.set_editor_property("intensity", 200000.0)
        lc.set_editor_property("attenuation_radius", 5000.0)
        lc.set_editor_property("light_color", unreal.Color(220, 230, 255, 255))
        lc.set_editor_property("cast_shadows", False)
        lxf = light.add_track(unreal.MovieScene3DTransformTrack).add_section()
        lxf.set_range(0, int(end_time * FPS))
        lch = channels(lxf)
        for c, v in zip(lch[0:3], [tip.x, tip.y, tip.z + 100.0]):
            c.add_key(frame(0), v)

        # on, off, on, off: the same double flicker as the game
        for b in (bolt, light):
            # a spawnable comes with its own spawn track, spawned throughout; key that one
            spawn = b.find_tracks_by_exact_type(unreal.MovieSceneSpawnTrack)
            sp = spawn[0].get_sections()[0] if spawn else b.add_track(unreal.MovieSceneSpawnTrack).add_section()
            sp.set_range(0, int(end_time * FPS))
            sc = channels(sp)[0]
            sc.add_key(frame(0), False)
            for k, on in enumerate([True, False, True, False]):
                sc.add_key(frame(t + 0.1 * k), on)

        thunder = audio.add_section()
        thunder.set_sound(eal.load_asset(THUNDER[n % len(THUNDER)]))
        thunder.set_range(frame(t + delay).value, frame(min(t + delay + 6.0, end_time)).value)
        lines.append("%s at %.1fs, bolt %.0f m, thunder %.1fs later" % (label, t, length / 100.0, delay))

    eal.save_loaded_asset(seq, False)
    lines.append("saved %s, %d camera keys, %.0f s" % (path, len(keys), end_time))
except Exception as e:
    import traceback
    lines.append("ERROR %r\n%s" % (e, traceback.format_exc()))
with open(LOG, "w") as f:
    f.write("\n".join(lines))
