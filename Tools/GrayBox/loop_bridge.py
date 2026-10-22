"""Core -> Level1 bridge as a self-crossing loop: the outgoing leg passes under X, circles, then crosses over X to Level1.
World cm at the 0.5 Rhino scale. Output rows like bridges.py."""
import json, math

P0 = (797.0, -3060.0)      # core-side end of the existing bridge (z 100)
P1 = (-230.0, -3792.0)     # Level1-side end (z 1100)
Z0, Z1 = 100.0, 1100.0
X_DIST = 1100.0            # crossing point, measured from P0 along the outgoing leg
R = 450.0                  # loop radius (deck centreline)
WIDTH = 2.5                # m
T = 0.5                    # deck thickness (m)
STEP = 100.0               # cm between centreline samples
LAND = 100.0               # flat landing at each end


def norm(v):
    L = math.hypot(*v)
    return (v[0] / L, v[1] / L)


dA = norm((-297.0, -1480.0))                      # current outgoing direction
X = (P0[0] + dA[0] * X_DIST, P0[1] + dA[1] * X_DIST)
dB = norm((P1[0] - X[0], P1[1] - X[1]))           # direction of the final leg through X
u1, u2 = dA, (-dB[0], -dB[1])                     # the loop sits in the angle between these rays
theta = math.acos(u1[0] * u2[0] + u1[1] * u2[1])
bis = norm((u1[0] + u2[0], u1[1] + u2[1]))
C = (X[0] + bis[0] * R / math.sin(theta / 2), X[1] + bis[1] * R / math.sin(theta / 2))
tdist = R / math.tan(theta / 2)
T1 = (X[0] + u1[0] * tdist, X[1] + u1[1] * tdist)
T2 = (X[0] + u2[0] * tdist, X[1] + u2[1] * tdist)


def line(a, b):
    n = max(1, round(math.dist(a, b) / STEP))
    return [(a[0] + (b[0] - a[0]) * i / n, a[1] + (b[1] - a[1]) * i / n) for i in range(n)]


a1 = math.atan2(T1[1] - C[1], T1[0] - C[0])
a2 = math.atan2(T2[1] - C[1], T2[0] - C[0])
# go the long way round (the major arc), in the direction that continues smoothly from u1
cross = u1[0] * (T1[1] - C[1]) - u1[1] * (T1[0] - C[0])
sgn = 1 if cross < 0 else -1          # +1: increasing angle
sweep = (a2 - a1) % (2 * math.pi) if sgn > 0 else -((a1 - a2) % (2 * math.pi))
narc = max(8, round(abs(sweep) * R / STEP))
arc = [(C[0] + R * math.cos(a1 + sweep * i / narc), C[1] + R * math.sin(a1 + sweep * i / narc)) for i in range(narc)]


def ext(p, q, d=100.0):
    v = norm((p[0] - q[0], p[1] - q[1]))
    return (p[0] + v[0] * d, p[1] + v[1] * d)


pts = [ext(P0, X)] + line(P0, T1) + arc + line(T2, P1) + [P1, ext(P1, X)]
s = [0.0]
for a, b in zip(pts, pts[1:]):
    s.append(s[-1] + math.dist(a, b))
L = s[-1]


def zat(si):
    if si <= LAND: return Z0
    if si >= L - LAND: return Z1
    return Z0 + (Z1 - Z0) * (si - LAND) / (L - 2 * LAND)


rows = []
for i, (a, b) in enumerate(zip(pts, pts[1:])):
    za, zb = zat(s[i]), zat(s[i + 1])
    dh = math.dist(a, b)
    rows.append([f"GB_Bridge_CoreToLevel1_{i+1:02d}", "Bridges", "floor",
                 round((a[0] + b[0]) / 2, 1), round((a[1] + b[1]) / 2, 1), round((za + zb) / 2 - T * 50, 1),
                 round(math.hypot(dh, zb - za) / 100 * 1.15, 3), WIDTH, T,
                 round(math.degrees(math.atan2(b[1] - a[1], b[0] - a[0])), 2),
                 round(math.degrees(math.atan2(zb - za, dh)), 2), "Cube"])

# deck height at the two passes over X
sx = [s[i] for i, p in enumerate(pts) if math.dist(p, X) < STEP * 0.6]
passes = sorted({round(zat(v)) for v in sx})
grade = math.degrees(math.atan2(Z1 - Z0, L - 2 * LAND))
json.dump(rows, open("loop_bridge_ue.json", "w"), separators=(",", ":"))
print(f"X={X[0]:.0f},{X[1]:.0f} C={C[0]:.0f},{C[1]:.0f} theta={math.degrees(theta):.0f} sweep={math.degrees(sweep):.0f}")
print(f"length {L/100:.1f} m, grade {grade:.1f} deg, deck z over X: {passes}, blocks {len(rows)}")
