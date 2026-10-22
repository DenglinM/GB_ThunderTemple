"""Port of render.py geometry -> list of boxes (meters, render coords). Prints JSON."""
import json

boxes = []  # dict(zone,name,c,x0,y0,z0,x1,y1,z1)
ZONE = [""]


def box(x0, y0, x1, y1, z0=0.0, z1=1.0, c="wall", name="Box"):
    boxes.append(dict(zone=ZONE[0], name=name, c=c, x0=min(x0, x1), y0=min(y0, y1), z0=z0,
                      x1=max(x0, x1), y1=max(y0, y1), z1=z1))


def _segs(a0, a1, gaps):
    segs = [(a0, a1)]
    for a, b in gaps:
        new = []
        for s, e in segs:
            if b <= s or a >= e:
                new.append((s, e))
            else:
                if a > s:
                    new.append((s, a))
                if b < e:
                    new.append((b, e))
        segs = new
    return segs


def wall_x(x, y0, y1, h, t=0.6, gaps=(), c="wall", name="Wall"):
    for s, e in _segs(y0, y1, gaps):
        box(x - t / 2, s, x + t / 2, e, 0, h, c, name)


def wall_y(y, x0, x1, h, t=0.6, gaps=(), c="wall", name="Wall"):
    for s, e in _segs(x0, x1, gaps):
        box(s, y - t / 2, e, y + t / 2, 0, h, c, name)


def decal(x0, y0, x1, y1, c, h=0.06, name="Floor"):
    box(x0, y0, x1, y1, 0, h, c, name)


def stalker(x, y, name):
    box(x - 0.45, y - 0.45, x + 0.45, y + 0.45, 0, 1.3, "enemy", name + "_Legs")
    box(x - 0.6, y - 0.35, x + 0.6, y + 0.35, 1.3, 2.0, "enemy", name + "_Body")
    box(x - 0.3, y - 0.3, x + 0.3, y + 0.3, 2.0, 2.5, "enemy", name + "_Head")


def arange(a, b, s):
    out, v = [], a
    while v < b - 1e-9:
        out.append(v)
        v += s
    return out


ZONE[0] = "00_Ground"
box(-19, -4, 19, 92, -0.6, 0, "floor", "Ground")

ZONE[0] = "01_Gate_Tutorial"
wall_x(-6, 0, 14, 3.0)
wall_x(6, 0, 14, 3.0)
decal(-2.5, 1, 2.5, 4.5, "green", 0.12, "Spawn_Save")
box(4.0, 5.5, 5.2, 6.0, 0, 1.6, "blue", "Tutorial_Stele")
for xp in (-3.2, 3.2):
    box(xp - 0.45, 7.6, xp + 0.45, 8.5, 0, 5.0, name="Gate_Pillar")
box(-4.4, 7.6, 4.4, 8.5, 5.0, 5.8, name="Gate_Lintel")
box(-3.6, 7.75, 3.6, 8.35, 4.3, 4.7, name="Gate_Beam")
decal(-4.5, 10.0, 4.5, 12.0, "red", name="Noise_Tutorial")

ZONE[0] = "02_FrontHall_Stealth"
wall_y(14, -10, 10, 3.0, gaps=[(-2, 2)])
wall_y(34, -10, 10, 3.0, gaps=[(-2.5, 2.5)])
wall_x(-10, 14, 34, 4.0)
wall_x(10, 14, 34, 4.0)
decal(-9.7, 14.3, 9.7, 33.7, "floor", 0.02, "HallFloor")
for (px, py) in [(-5, 19), (5, 19), (-5, 28.5), (5, 28.5)]:
    box(px - 0.7, py - 0.7, px + 0.7, py + 0.7, 0, 2.5, name="FullCover_Pillar")
box(-1.8, 23.2, 1.8, 24.2, 0, 1.0, name="CrouchCover")
box(6.6, 24.5, 7.6, 27.5, 0, 1.0, name="CrouchCover")
decal(-9.7, 20, -6.2, 26.5, "red", name="Noise")
decal(1.5, 25.5, 9.7, 27.0, "red", name="Noise")
decal(-3.5, 15.5, 3.5, 17.0, "red", name="Noise")
decal(-9.7, 30.5, -2.5, 33.0, "red", name="Noise")
box(-8.6, 16.0, -8.0, 16.6, 0, 0.6, "blue", "BellShard")
box(7.8, 31.0, 8.4, 31.6, 0, 0.6, "blue", "BellShard")
stalker(-1.0, 27.5, "Stalker1")
decal(-1.5, 31.6, 1.5, 33.4, "green", 0.12, "Checkpoint1")

ZONE[0] = "03_Corridor_NoiseTiles"
wall_x(-3.2, 34, 44, 3.5, gaps=[(38, 40.5)])
wall_x(3.2, 34, 44, 3.5)
for i, yy in enumerate(arange(34.3, 43.7, 1.3)):
    for j, xx in enumerate(arange(-2.9, 2.9, 1.45)):
        if (i + j) % 2 == 0:
            decal(xx, yy, xx + 1.45, min(yy + 1.3, 43.7), "red", name="NoiseTile")
box(-6.0, 37.7, -3.5, 40.8, 0, 0.02, "floor", "NicheFloor")
wall_x(-6.2, 37.5, 41, 3.5)
wall_y(37.5, -6.5, -3.2, 3.5)
wall_y(41.0, -6.5, -3.2, 3.5)
decal(-5.8, 38.0, -3.6, 40.5, "green", 0.12, "SafeNiche")

ZONE[0] = "04_RodCourtyard_Combat"
wall_y(44, -14, 14, 2.0, gaps=[(-3.2, 3.2)])
wall_y(62, -14, 14, 2.0, gaps=[(-3.0, 3.0)])
wall_x(-14, 44, 62, 2.5)
wall_x(14, 44, 62, 2.5)
decal(-13.7, 44.3, 13.7, 61.7, "floor", 0.02, "CourtFloor")
decal(-1.5, 44.6, 1.5, 46.4, "green", 0.12, "Checkpoint2")
for k, (px, py) in enumerate([(-8, 51), (8, 51), (0, 57.5)]):
    decal(px - 2.4, py - 2.4, px + 2.4, py + 2.4, "yellow", 0.08, f"CopperPlate{k+1}")
    box(px - 0.9, py + 2.4, px + 0.9, py + 3.6, 0, 0.8, name=f"Rod{k+1}_Base")
    box(px - 0.3, py + 2.7, px + 0.3, py + 3.3, 0.8, 8.5, "yellow", f"Rod{k+1}_Pole")
    box(px - 0.6, py + 2.4, px + 0.6, py + 3.6, 8.5, 9.0, "yellow", f"Rod{k+1}_Cap")
stalker(5.5, 49.0, "Stalker2")
for (px, py) in [(-11.5, 46.5), (11.0, 59.5), (-4.0, 47.5)]:
    box(px - 1.5, py - 0.5, px + 1.5, py + 0.5, 0, 1.0, name="CrouchCover")
box(-12.4, 58.6, -11.8, 59.2, 0, 0.6, "blue", "BellShard")
box(3.8, 46.2, 4.4, 46.8, 0, 0.6, "blue", "BellShard")

ZONE[0] = "05_Plaza_CountPuzzle"
for k, xp in enumerate((-4, 0, 4)):
    box(xp - 0.6, 65.4, xp + 0.6, 66.6, 0, 1.0, name=f"BellStand{k+1}")
    box(xp - 0.4, 65.6, xp + 0.4, 66.4, 1.0, 1.7, "blue", f"PuzzleBell{k+1}")
for nm, (px, py, ph) in [("Near", (-12.5, 66, 6.0)), ("Mid", (15.5, 70, 9.0)), ("Far", (-16.5, 86, 12.0))]:
    box(px - 1.3, py - 1.3, px + 1.3, py + 1.3, 0, 1.0, name=f"Pillar{nm}_Base")
    box(px - 0.4, py - 0.4, px + 0.4, py + 0.4, 1.0, ph, "yellow", f"Pillar{nm}_Pole")
    box(px - 0.75, py - 0.75, px + 0.75, py + 0.75, ph, ph + 0.5, "yellow", f"Pillar{nm}_Cap")

ZONE[0] = "06_MainHall_Altar"
wall_y(70, -10, 10, 5.0, gaps=[(-2.2, 2.2)])
box(-2.2, 69.75, 2.2, 70.25, 0, 4.2, "blue", "SealDoor")
box(-2.6, 69.6, 2.6, 70.4, 4.2, 5.0, name="SealDoor_Lintel")
wall_x(-10, 70, 86, 6.0)
wall_x(10, 70, 86, 6.0)
wall_y(86, -10, 10, 6.0)
decal(-9.7, 70.3, 9.7, 85.7, "floor", 0.02, "HallFloor")
for xp in (-6, 6):
    for yp in (74, 80):
        box(xp - 0.6, yp - 0.6, xp + 0.6, yp + 0.6, 0, 5.5, name="HallPillar")
box(-4, 76, 4, 84, 0, 0.5, "green", "Altar_Tier1")
box(-2.8, 77.2, 2.8, 82.8, 0.5, 1.0, "green", "Altar_Tier2")
box(-1.4, 78.6, 1.4, 81.4, 1.0, 1.6, "green", "Altar_Tier3")
box(-0.3, 79.7, 0.3, 80.3, 1.6, 5.0, "yellow", "Altar_Rod")

print(json.dumps(boxes))
