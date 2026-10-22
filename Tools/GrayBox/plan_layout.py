"""Hand-drawn plan (plan.jpg, 2000px-wide view) -> UE gray box blocks.
Plan px -> world: origin at goal centre (750,330); 1 px = 0.1 m; up in image = UE +X, right = UE +Y.
Output rows: [label, folder, mat, x, y, z, sx, sy, sz, yaw, pitch, mesh] (cm / scale in m / deg)."""
import json, math

S = 0.1          # m per px
O = (750, 330)   # goal centre in px
Z_B1, Z_CORE, Z_LVL, Z_META = -4.0, 0.0, 10.0, 13.0
T = 1.0          # platform thickness (m)

rows = []
cnt = {}


def w(p):  # px -> (X, Y) metres
    return ((O[1] - p[1]) * S, (p[0] - O[0]) * S)


def add(name, folder, mat, cx, cy, cz, sx, sy, sz, yaw=0.0, pitch=0.0, mesh="Cube"):
    cnt[name] = cnt.get(name, 0) + 1
    rows.append([f"GB_{name}_{cnt[name]:02d}", folder, mat, round(cx * 100, 1), round(cy * 100, 1),
                 round(cz * 100, 1), round(sx, 3), round(sy, 3), round(sz, 3), round(yaw, 2), round(pitch, 2),
                 mesh])


def ramp(pts_px, z0, z1):
    """Assign heights along a px polyline, interpolated by horizontal distance."""
    d = [0.0]
    for a, b in zip(pts_px, pts_px[1:]):
        d.append(d[-1] + math.dist(a, b))
    return [(u, v, z0 + (z1 - z0) * di / d[-1]) for (u, v), di in zip(pts_px, d)]


def helix(name, folder, centre_px, r, a0, sweep, z0, z1, width=2.5, n=40, wall_h=2.5):
    """Spiral corridor: walkway segments + outer wall + central column. Angles in deg, world frame."""
    cx, cy = w(centre_px)
    pt = lambda a, rr: (cx + rr * math.cos(math.radians(a)), cy + rr * math.sin(math.radians(a)))
    for i in range(n):
        a, b = a0 + sweep * i / n, a0 + sweep * (i + 1) / n
        za, zb = z0 + (z1 - z0) * i / n, z0 + (z1 - z0) * (i + 1) / n
        (xa, ya), (xb, yb) = pt(a, r), pt(b, r)
        dh = math.hypot(xb - xa, yb - ya)
        yaw = math.degrees(math.atan2(yb - ya, xb - xa))
        pitch = math.degrees(math.atan2(zb - za, dh))
        add(name, folder, "floor", (xa + xb) / 2, (ya + yb) / 2, (za + zb) / 2 - T / 2,
            math.hypot(dh, zb - za) * 1.1, width, T, yaw, pitch)
        if i < 2 or i >= n - 2:   # leave openings where the entry/exit connectors attach
            continue
        (wa, wb) = pt(a, r + width / 2 + 0.2), pt(b, r + width / 2 + 0.2)
        add(name + "Wall", folder, "wall", (wa[0] + wb[0]) / 2, (wa[1] + wb[1]) / 2, (za + zb) / 2 + wall_h / 2,
            math.hypot(wb[0] - wa[0], wb[1] - wa[1]) * 1.05, 0.4, wall_h, yaw)
    col_r = r - width / 2 - 0.3
    add(name + "Column", folder, "wall", cx, cy, (z0 + z1 + wall_h) / 2, col_r * 2, col_r * 2,
        z1 + wall_h - z0, mesh="Cylinder")
    return pt(a0, r), pt(a0 + sweep, r)


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
F = "04_SpiralAscent"
ENTRY, HC, HR = (585, 480), (552, 400), 6.0       # entrance kept where the old ramp started
ex, ey = w(ENTRY)
hx, hy = w(HC)
A0 = math.degrees(math.atan2(ey - hy, ex - hx))
EXIT_TO = (510, 322)                                # landing on Level1
lx, ly = w(EXIT_TO)
A1 = math.degrees(math.atan2(ly - hy, lx - hx)) % 360
SWEEP = (A1 - A0) % 360 + 360                       # ~1.5 turns
(sx0, sy0), (sx1, sy1) = helix("Spiral", F, HC, HR, A0, SWEEP, Z_CORE, Z_LVL)
to_px = lambda x, y: (O[0] + y / S, O[1] - x / S)
path("SpiralEntry", F, [(*ENTRY, Z_CORE), (*to_px(sx0, sy0), Z_CORE)], width=2.5)
path("SpiralExit", F, [(*to_px(sx1, sy1), Z_LVL), (*EXIT_TO, Z_LVL)], width=2.5)

# --- Level 1-3 (upper floor), linking paths, CPs ------------------------------
F = "05_Level1"
platform("Level1", F, [(450, 250), (500, 235), (532, 250), (530, 312), (495, 330), (472, 370), (465, 420),
                       (450, 465), (410, 465), (385, 440), (370, 390), (390, 330), (420, 280)], Z_LVL)
meta = ramp([(470, 245), (465, 185), (490, 140), (530, 112)], Z_LVL, Z_META)
path("ToMeta", F, meta, width=2.0)
pad("CP2", F, (465, 185), meta[1][2], size=2.0)
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
down = ramp([(1085, 395), (1100, 330), (1085, 270), (1040, 240), (990, 235), (955, 255), (945, 282)],
            Z_LVL, Z_CORE)
path("L3toGoal", F, down, width=2.5)
pad("CP5", F, (1040, 240), down[3][2], size=2.5)

# --- Meta level area placeholder ---------------------------------------------
F = "08_MetaArea"
platform("MetaPlatform", F, [(515, 95), (550, 92), (570, 110), (560, 135), (530, 140), (512, 120)], Z_META)

print(json.dumps(rows, separators=(",", ":")))
