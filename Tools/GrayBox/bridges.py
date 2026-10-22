"""Fit gray-box bridges to the Rhino bridge top surfaces (mesh_9) and regrade them to a uniform walkable slope.
Output rows like plan_layout: [label, folder, mat, x, y, z, sx, sy, sz, yaw, pitch, mesh] (cm, scale in m, deg)."""
import json, math
from collections import deque

CELL = 25.0          # cm grid for sampling the ribbon top surface
BIN = 100.0          # cm of arc length per centerline point
T = 0.5              # bridge thickness (m)

comps = json.load(open("mesh9_comps.json"))

# (name, comp index of the top surface, start platform top z, end platform top z)  -- z in cm, after 0.5 scale
BRIDGES = [
    ("Bridge_StartToTutorial", 10, -596, -200),
    ("Bridge_TutorialToCore", 8, -200, 100),
    ("Bridge_CoreToLevel1", 0, 100, 1100),
    ("Bridge_Level1ToLevel2", 4, 1100, 1400),
    ("Bridge_Level2ToLevel3", 2, 1400, 1100),
    ("Bridge_Level3ToCore", 6, 1100, 100),
]


def raster(tris):
    """Top-surface heightfield: cell -> max z over the ribbon."""
    hf = {}
    for a, b, c in tris:
        xs, ys = [a[0], b[0], c[0]], [a[1], b[1], c[1]]
        area = (b[0] - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (b[1] - a[1])
        if abs(area) < 1e-6:
            continue
        for i in range(math.floor(min(xs) / CELL), math.floor(max(xs) / CELL) + 1):
            for j in range(math.floor(min(ys) / CELL), math.floor(max(ys) / CELL) + 1):
                px, py = (i + 0.5) * CELL, (j + 0.5) * CELL
                w0 = ((b[0] - px) * (c[1] - py) - (c[0] - px) * (b[1] - py)) / area
                w1 = ((c[0] - px) * (a[1] - py) - (a[0] - px) * (c[1] - py)) / area
                w2 = 1 - w0 - w1
                if min(w0, w1, w2) >= -1e-6:
                    z = w0 * a[2] + w1 * b[2] + w2 * c[2]
                    hf[(i, j)] = max(hf.get((i, j), -1e9), z)
    return hf


def centerline(hf, z_start):
    """Geodesic distance from the end nearest z_start; centroid per BIN of distance."""
    seed = min(hf, key=lambda k: abs(hf[k] - z_start))
    # seed may sit mid-width: restart BFS from the farthest cell of a first pass to reach the true end
    def bfs(src):
        dist = {src: 0}
        q = deque([src])
        while q:
            k = q.popleft()
            for di in (-1, 0, 1):
                for dj in (-1, 0, 1):
                    n = (k[0] + di, k[1] + dj)
                    if n in hf and n not in dist:
                        dist[n] = dist[k] + math.hypot(di, dj) * CELL
                        q.append(n)
        return dist
    d1 = bfs(seed)
    far = max(d1, key=d1.get)
    d2 = bfs(far)
    other = max(d2, key=d2.get)
    # orient so the line starts at the end whose height is closer to z_start
    if abs(hf[far] - z_start) > abs(hf[other] - z_start):
        d2 = bfs(other)
    bins = {}
    for k, dv in d2.items():
        bins.setdefault(int(dv // BIN), []).append(k)
    pts, widths = [], []
    for b in sorted(bins):
        ks = bins[b]
        x = sum((k[0] + 0.5) * CELL for k in ks) / len(ks)
        y = sum((k[1] + 0.5) * CELL for k in ks) / len(ks)
        pts.append((x, y))
        widths.append(len(ks) * CELL * CELL / BIN)
    return pts, widths


def smooth(pts, n=2):
    out = list(pts)
    for _ in range(n):
        out = [out[0]] + [((a[0] + b[0] + c[0]) / 3, (a[1] + b[1] + c[1]) / 3)
                          for a, b, c in zip(out, out[1:], out[2:])] + [out[-1]]
    return out


PLATFORMS = ["mesh_2", "mesh_16", "mesh_19", "mesh_17", "mesh_15", "mesh_18"]
ptris = []
for lab in PLATFORMS:
    for a, b, c in json.load(open("rhino_tris.json"))[lab]:
        n = (b[0] - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (b[1] - a[1])
        if abs(n) > 1 and a[2] == b[2] == c[2]:
            ptris.append((a, b, c))


def platform_top(x, y):
    """Top z of a Rhino platform under (x, y), or None."""
    best = None
    for a, b, c in ptris:
        area = (b[0] - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (b[1] - a[1])
        w0 = ((b[0] - x) * (c[1] - y) - (c[0] - x) * (b[1] - y)) / area
        w1 = ((c[0] - x) * (a[1] - y) - (a[0] - x) * (c[1] - y)) / area
        if min(w0, w1, 1 - w0 - w1) >= 0:
            best = a[2] if best is None else max(best, a[2])
    return best


rows, report = [], []
for name, ci, z0, z1 in BRIDGES:
    hf = raster(comps[ci])
    pts, widths = centerline(hf, z0)
    pts = smooth(pts)
    # extend 1 m past both ends so the deck overlaps the platforms
    def ext(p, q):
        L = math.dist(p, q) or 1
        return (p[0] + (p[0] - q[0]) / L * 100, p[1] + (p[1] - q[1]) / L * 100)
    pts = [ext(pts[0], pts[1])] + pts + [ext(pts[-1], pts[-2])]
    s = [0.0]
    for a, b in zip(pts, pts[1:]):
        s.append(s[-1] + math.dist(a, b))
    L = s[-1]
    # grade only over the stretch that is not above the source/destination platform (sampled every 25 cm)
    s_lo, s_hi, k = 0.0, L, 0
    for i, (a, b) in enumerate(zip(pts, pts[1:])):
        seg = math.dist(a, b)
        for f in [j / 8 for j in range(8)]:
            x, y = a[0] + (b[0] - a[0]) * f, a[1] + (b[1] - a[1]) * f
            top = platform_top(x, y)
            si = s[i] + seg * f
            if top is not None and abs(top - z0) < 5 and si < L / 2:
                s_lo = max(s_lo, si)
            if top is not None and abs(top - z1) < 5 and si > L / 2:
                s_hi = min(s_hi, si)
    width = max(200.0, sorted(widths)[len(widths) // 2]) / 100
    # hairpins: where the deck folds back over itself, keep that stretch flat so the overlap has no ledge
    flat = [False] * (len(pts) - 1)
    for i in range(len(pts)):
        for j in range(i + int(width * 100 / BIN) + 3, len(pts)):
            if math.dist(pts[i], pts[j]) < width * 100:
                for k in range(i, j):
                    flat[k] = True
    eff = [0.0]
    for k, (a, b) in enumerate(zip(pts, pts[1:])):
        lo, hi = max(s[k], s_lo), min(s[k + 1], s_hi)
        eff.append(eff[-1] + (0 if flat[k] else max(0.0, hi - lo)))

    def zat_i(i):
        return z0 + (z1 - z0) * eff[i] / eff[-1]
    s_lo, s_hi = 0.0, eff[-1]       # report grade over the sloped part only

    def zat(si):
        i = min(range(len(s)), key=lambda k: abs(s[k] - si))
        return zat_i(i)
    grade = math.degrees(math.atan2(abs(z1 - z0), s_hi - s_lo))
    report.append(f"{name}: length {L/100:.1f} m, width {width:.1f} m, rise {(z1-z0)/100:+.1f} m, grade {grade:.1f} deg")
    for i, (a, b) in enumerate(zip(pts, pts[1:])):
        za, zb = zat(s[i]), zat(s[i + 1])
        dh = math.dist(a, b)
        yaw = math.degrees(math.atan2(b[1] - a[1], b[0] - a[0]))
        pitch = math.degrees(math.atan2(zb - za, dh))
        rows.append([f"GB_{name}_{i+1:02d}", "Bridges", "floor", round((a[0] + b[0]) / 2, 1), round((a[1] + b[1]) / 2, 1),
                     round((za + zb) / 2 - T * 50, 1), round(math.hypot(dh, zb - za) / 100 * 1.2, 3), round(width, 3), T,
                     round(yaw, 2), round(pitch, 2), "Cube"])

json.dump(rows, open("bridges_ue.json", "w"), separators=(",", ":"))
print("\n".join(report))
print("blocks", len(rows))
