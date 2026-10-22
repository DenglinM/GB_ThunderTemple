"""Hand-drawn plan (plan.jpg, 2000px-wide view) -> UE gray box blocks.
Plan px -> world: origin at goal centre (750,330); 1 px = 0.1 m; up in image = UE +X, right = UE +Y.
Output rows: [label, folder, mat, x, y, z, sx, sy, sz, yaw, pitch] (cm / scale in m / deg)."""
import json, math

S = 0.1          # m per px
O = (750, 330)   # goal centre in px
Z_B1, Z_CORE, Z_LVL, Z_META = -4.0, 0.0, 6.0, 9.0
T = 1.0          # platform thickness (m)

rows = []
cnt = {}


def w(p):  # px -> (X, Y) metres
    return ((O[1] - p[1]) * S, (p[0] - O[0]) * S)


def add(name, folder, mat, cx, cy, cz, sx, sy, sz, yaw=0.0, pitch=0.0):
    cnt[name] = cnt.get(name, 0) + 1
    rows.append([f"GB_{name}_{cnt[name]:02d}", folder, mat, round(cx * 100, 1), round(cy * 100, 1),
                 round(cz * 100, 1), round(sx, 3), round(sy, 3), round(sz, 3), round(yaw, 2), round(pitch, 2)])


def platform(name, folder, poly_px, ztop, mat="floor", step=1.0):
    """Scanline-fill polygon with 1 m strips running along X."""
    P = [w(p) for p in poly_px]
    ys = [p[1] for p in P]
    y = math.floor(min(ys)) + step / 2
    while y < max(ys):
        xs = []
        for (x0, y0), (x1, y1) in zip(P, P[1:] + P[:1]):
            if (y0 <= y < y1) or (y1 <= y < y0):
                xs.append(x0 + (y - y0) * (x1 - x0) / (y1 - y0))
        xs.sort()
        for a, b in zip(xs[0::2], xs[1::2]):
            if b - a > 0.3:
                add(name, folder, mat, (a + b) / 2, y, ztop - T / 2, b - a, step, T)
        y += step


def path(name, folder, pts, width=3.0, mat="floor"):
    """pts: [(u, v, ztop)] in px; sloped segments + square caps at joints."""
    W = [(*w((u, v)), z) for u, v, z in pts]
    for (x0, y0, z0), (x1, y1, z1) in zip(W, W[1:]):
        dh = math.hypot(x1 - x0, y1 - y0)
        L = math.hypot(dh, z1 - z0)
        yaw = math.degrees(math.atan2(y1 - y0, x1 - x0))
        pitch = math.degrees(math.atan2(z1 - z0, dh))
        add(name, folder, mat, (x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2 - T / 2, L, width, T, yaw, pitch)
    for x, y, z in W[1:-1]:
        add(name + "Joint", folder, mat, x, y, z - T / 2, width, width, T)


def bar(name, folder, a_px, b_px, ztop, h=3.0, t=1.0, mat="wall"):
    (x0, y0), (x1, y1) = w(a_px), w(b_px)
    L = math.hypot(x1 - x0, y1 - y0)
    add(name, folder, mat, (x0 + x1) / 2, (y0 + y1) / 2, ztop + h / 2, L, t, h,
        math.degrees(math.atan2(y1 - y0, x1 - x0)))


def pad(name, folder, c_px, ztop, size=3.0, mat="green", h=0.12):
    x, y = w(c_px)
    add(name, folder, mat, x, y, ztop + h / 2, size, size, h)


# --- B1 start + tutorial bridge -------------------------------------------
F = "01_Start_B1"
platform("StartPlatform", F, [(410, 900), (470, 890), (520, 930), (540, 1000), (520, 1050),
                              (470, 1030), (420, 1005), (405, 950)], Z_B1)
pad("StartPoint", F, (450, 960), Z_B1)
F = "02_Bridge_Tutorial"
Z_TUT = -3.0
platform("TutorialPlatform", F, [(540, 840), (600, 820), (645, 835), (660, 875), (640, 915), (590, 930),
                                 (545, 915), (530, 880)], Z_TUT)
path("Bridge", F, [(480, 905, Z_B1), (530, 865, Z_TUT), (585, 820, Z_TUT), (605, 775, -2.1),
                   (612, 715, -1.4), (625, 650, -0.7), (640, 590, -0.15), (655, 545, Z_CORE)], width=2.5)
pad("CP1", F, (640, 590), -0.15, size=2.5)

# --- Core / goal space -------------------------------------------------------
F = "03_Core_Goal"
core = [(700, 150), (760, 148), (820, 155), (880, 172), (905, 200), (935, 250), (950, 290), (960, 330),
        (955, 380), (940, 420), (920, 470), (890, 520), (850, 552), (800, 565), (760, 568), (700, 560),
        (660, 552), (620, 530), (580, 520), (550, 500), (545, 460), (548, 400), (548, 330), (550, 270),
        (565, 230), (595, 190), (640, 162)]
platform("CoreFloor", F, core, Z_CORE)
pad("Goal", F, (745, 325), Z_CORE, size=4.0)
for a, b in [((730, 140), (790, 140)), ((670, 225), (840, 225)), ((672, 285), (672, 350)),
             ((817, 272), (817, 310)), ((817, 375), (817, 440)), ((628, 378), (672, 488))]:
    bar("Obstacle", F, a, b, Z_CORE)
for c in [(865, 377), (845, 465)]:
    x, y = w(c)
    add("CorePillar", F, "wall", x, y, Z_CORE + 1.0, 1.2, 1.2, 2.0)

# --- Ascent to Level1 --------------------------------------------------------
F = "04_Ascent"
path("Ascent", F, [(585, 480, Z_CORE), (565, 400, 3.0), (558, 320, Z_LVL), (528, 300, Z_LVL)], width=2.5)

# --- Level 1-3 (2F), linking paths, CPs ---------------------------------------
F = "05_Level1"
platform("Level1", F, [(450, 250), (500, 235), (532, 250), (532, 320), (510, 380), (480, 430),
                       (450, 465), (410, 465), (385, 440), (370, 390), (390, 330), (420, 280)], Z_LVL)
path("ToMeta", F, [(470, 245, Z_LVL), (465, 185, 7.0), (490, 140, 8.0), (530, 112, Z_META)], width=2.0)
pad("CP2", F, (465, 185), 7.0, size=2.0)
F = "06_Level2"
path("L1toL2", F, [(430, 462, Z_LVL), (440, 520, Z_LVL), (455, 570, Z_LVL), (460, 605, Z_LVL)], width=2.5)
pad("CP3", F, (455, 570), Z_LVL, size=2.5)
platform("Level2", F, [(425, 620), (470, 595), (520, 585), (560, 590), (600, 600), (650, 590), (700, 600),
                       (750, 620), (790, 660), (790, 700), (760, 740), (700, 745), (640, 740), (590, 760),
                       (550, 760), (510, 745), (470, 720), (430, 690), (420, 650)], Z_LVL)
F = "07_Level3"
path("L2toL3", F, [(785, 685, Z_LVL), (840, 680, Z_LVL), (880, 665, Z_LVL), (905, 640, Z_LVL)], width=2.5)
pad("CP4", F, (880, 665), Z_LVL, size=2.5)
platform("Level3", F, [(890, 620), (920, 560), (960, 500), (1000, 450), (1030, 400), (1080, 385),
                       (1130, 410), (1140, 460), (1135, 530), (1120, 600), (1060, 640), (1000, 665),
                       (950, 670), (900, 650)], Z_LVL)
path("L3toGoal", F, [(1085, 395, Z_LVL), (1088, 340, Z_LVL), (1045, 300, 4.0), (1000, 285, 2.0),
                     (952, 280, Z_CORE)], width=2.5)
pad("CP5", F, (1045, 300), 4.0, size=2.5)

# --- Meta level area placeholder ---------------------------------------------
F = "08_MetaArea"
platform("MetaPlatform", F, [(515, 95), (550, 92), (570, 110), (560, 135), (530, 140), (512, 120)], Z_META)

print(json.dumps(rows, separators=(",", ":")))
