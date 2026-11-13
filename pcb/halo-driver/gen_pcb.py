#!/usr/bin/env python3
"""Halo driver PCB generator (v1).

Writes Gerber + Excellon files and an SVG preview for the small 2-layer,
all-through-hole board that sits between the Waveshare ESP32-S3-Touch-AMOLED-1.75
header and the 24-LED RGBW NeoPixel ring. See docs/HARDWARE.md section 7.

Run:  python3 gen_pcb.py        (standard library only)
It runs its own clearance and connectivity checks and exits non-zero if they fail.
Coordinates are millimetres, origin at the board's top-left, y pointing down.
"""
import math, os, sys, zipfile

W, H = 44.0, 28.0           # board outline
CLEAR = 0.30                # minimum copper-to-copper gap, different nets
EDGE = 0.50                 # minimum copper-to-board-edge gap
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "gerbers")
NAME = "halo-driver"

# ---------------------------------------------------------------- pads
# (ref, pin, x, y, pad diameter, drill, net)
P = 2.54
pads = []
J1_NETS = ["VBUS", "GND", "NC_3V3", "NC_44", "NC_43", "DATA", "NC_18", "NC_16"]
for i, net in enumerate(J1_NETS):                       # J1: 1x8 header to the Waveshare board
    pads.append(("J1", i + 1, 3.0, 4.0 + i * P, 1.7, 1.0, net))
U1L = ["GND", "DATA", "Y1", "VBUS", "GND", "NC_U6", "GND"]            # pins 1..7
U1R = ["VBUS", "VBUS", "GND", "NC_U11", "VBUS", "GND", "NC_U8"]       # pins 14..8
for i, net in enumerate(U1L):                           # U1: 74AHCT125, DIP-14
    pads.append(("U1", i + 1, 14.0, 5.0 + i * P, 1.6, 0.8, net))
for i, net in enumerate(U1R):
    pads.append(("U1", 14 - i, 21.62, 5.0 + i * P, 1.6, 0.8, net))
pads += [
    ("R2", 1, 8.0, 7.54, 1.8, 0.8, "DATA"),  ("R2", 2, 8.0, 17.70, 1.8, 0.8, "GND"),    # 10k pull-down
    ("R1", 1, 11.2, 10.08, 1.8, 0.8, "Y1"),  ("R1", 2, 11.2, 20.24, 1.8, 0.8, "DIN"),   # 470R series
    ("C2", 1, 27.5, 5.0, 1.6, 0.8, "VBUS"),  ("C2", 2, 27.5, 7.54, 1.6, 0.8, "GND"),    # 100nF
    ("F1", 1, 30.42, 3.5, 2.0, 1.1, "VBUS"), ("F1", 2, 35.5, 3.5, 2.0, 1.1, "5VR"),     # PTC MF-R110
    ("C1", 1, 35.5, 13.0, 2.0, 1.0, "5VR"),  ("C1", 2, 30.42, 13.0, 2.0, 1.0, "GND"),   # 1000uF, pin 1 = +
    ("J2", 1, 30.42, 22.5, 1.8, 1.0, "GND"), ("J2", 2, 32.96, 22.5, 1.8, 1.0, "DIN"),
    ("J2", 3, 35.5, 22.5, 1.8, 1.0, "5VR"),                                              # J2: to the ring
]
holes = [(3.0, 25.3, 2.2), (41.0, 25.0, 2.2), (41.0, 3.0, 2.2)]   # M2 mounting, not plated

# ---------------------------------------------------------------- tracks
# (layer, net, width, [points])
T, B = "F", "B"
tracks = [
    (T, "VBUS", 1.0, [(3, 4.0), (3, 2.0), (30.42, 2.0), (30.42, 3.5)]),
    (T, "VBUS", 0.6, [(21.62, 2.0), (21.62, 5.0), (21.62, 7.54)]),
    (T, "VBUS", 0.6, [(21.62, 5.0), (27.5, 5.0)]),
    (T, "VBUS", 0.6, [(24.6, 5.0), (24.6, 15.16), (21.62, 15.16)]),
    (T, "VBUS", 0.6, [(21.62, 7.54), (17.8, 7.54), (17.8, 12.62), (14, 12.62)]),
    (T, "5VR", 1.0, [(35.5, 3.5), (35.5, 13.0), (35.5, 22.5)]),
    (T, "DATA", 0.4, [(3, 16.70), (5.5, 16.70), (5.5, 7.54), (8.0, 7.54), (14, 7.54)]),
    (T, "Y1", 0.4, [(14, 10.08), (11.2, 10.08)]),
    (T, "DIN", 0.4, [(11.2, 20.24), (11.2, 24.5), (32.96, 24.5), (32.96, 22.5)]),
    (B, "GND", 1.0, [(3, 6.54), (5.5, 6.54), (5.5, 22.5), (30.42, 22.5)]),
    (B, "GND", 1.0, [(30.42, 13.0), (30.42, 22.5)]),
    (B, "GND", 0.6, [(14, 5.0), (5.5, 5.0), (5.5, 6.54)]),
    (B, "GND", 0.6, [(27.5, 7.54), (27.5, 10.08), (21.62, 10.08)]),
    (B, "GND", 0.6, [(26.0, 10.08), (26.0, 17.70), (17.8, 17.70)]),
    (B, "GND", 0.6, [(17.8, 15.16), (17.8, 22.5)]),
    (B, "GND", 0.6, [(14, 15.16), (17.8, 15.16)]),
    (B, "GND", 0.6, [(14, 20.24), (17.8, 20.24)]),
    (B, "GND", 0.6, [(14, 15.16), (8.0, 15.16), (8.0, 17.70)]),
]
# the netlist the copper must implement: net -> set of (ref, pin)
NETLIST = {
    "VBUS": {("J1", 1), ("U1", 14), ("U1", 13), ("U1", 10), ("U1", 4), ("C2", 1), ("F1", 1)},
    "5VR":  {("F1", 2), ("C1", 1), ("J2", 3)},
    "GND":  {("J1", 2), ("U1", 1), ("U1", 5), ("U1", 7), ("U1", 12), ("U1", 9), ("R2", 2),
             ("C2", 2), ("C1", 2), ("J2", 1)},
    "DATA": {("J1", 6), ("R2", 1), ("U1", 2)},
    "Y1":   {("U1", 3), ("R1", 1)},
    "DIN":  {("R1", 2), ("J2", 2)},
}

# ---------------------------------------------------------------- geometry helpers
def d_pt_seg(p, a, b):
    (px, py), (ax, ay), (bx, by) = p, a, b
    dx, dy = bx - ax, by - ay
    L = dx * dx + dy * dy
    t = 0 if L == 0 else max(0, min(1, ((px - ax) * dx + (py - ay) * dy) / L))
    return math.hypot(px - (ax + t * dx), py - (ay + t * dy))

def seg_cross(a, b, c, d):
    def o(p, q, r): return (q[0] - p[0]) * (r[1] - p[1]) - (q[1] - p[1]) * (r[0] - p[0])
    return o(a, b, c) * o(a, b, d) < 0 and o(c, d, a) * o(c, d, b) < 0

def d_seg_seg(a, b, c, d):
    if seg_cross(a, b, c, d): return 0.0
    return min(d_pt_seg(a, c, d), d_pt_seg(b, c, d), d_pt_seg(c, a, b), d_pt_seg(d, a, b))

def segs(layer=None):
    for lay, net, w, pts in tracks:
        if layer and lay != layer: continue
        for a, b in zip(pts, pts[1:]):
            yield lay, net, w, a, b

# ---------------------------------------------------------------- checks
def check():
    errs = []
    # 1. netlist on the pads matches the intended netlist
    for net, members in NETLIST.items():
        got = {(r, p) for r, p, *_, n in pads if n == net}
        if got != members: errs.append(f"netlist mismatch on {net}: {sorted(got ^ members)}")
    # 2. clearance, per layer (through-hole pads exist on both layers)
    for lay in (T, B):
        S = list(segs(lay))
        for i, (_, n1, w1, a, b) in enumerate(S):
            for _, n2, w2, c, d in S[i + 1:]:
                if n1 != n2:
                    g = d_seg_seg(a, b, c, d) - w1 / 2 - w2 / 2
                    if g < CLEAR: errs.append(f"{lay}: track {n1} to track {n2} gap {g:.2f}")
            for r, p, x, y, dia, _, n2 in pads:
                if n1 != n2:
                    g = d_pt_seg((x, y), a, b) - w1 / 2 - dia / 2
                    if g < CLEAR: errs.append(f"{lay}: track {n1} to pad {r}.{p} gap {g:.2f}")
            for hx, hy, hd in holes:
                g = d_pt_seg((hx, hy), a, b) - w1 / 2 - hd / 2
                if g < CLEAR: errs.append(f"{lay}: track {n1} to mounting hole gap {g:.2f}")
            for x, y in (a, b):
                if min(x, y, W - x, H - y) - w1 / 2 < EDGE: errs.append(f"{lay}: track {n1} too near edge")
    for i, (r1, p1, x1, y1, d1, _, n1) in enumerate(pads):
        for r2, p2, x2, y2, d2, _, n2 in pads[i + 1:]:
            g = math.hypot(x1 - x2, y1 - y2) - d1 / 2 - d2 / 2
            if n1 != n2 and g < CLEAR: errs.append(f"pad {r1}.{p1} to pad {r2}.{p2} gap {g:.2f}")
        for hx, hy, hd in holes:
            if math.hypot(x1 - hx, y1 - hy) - d1 / 2 - hd / 2 < CLEAR: errs.append(f"pad {r1}.{p1} near hole")
        if min(x1, y1, W - x1, H - y1) - d1 / 2 < EDGE: errs.append(f"pad {r1}.{p1} too near edge")
    # 3. connectivity: every net's pads joined by copper, and no track end left hanging
    parent = {}
    def find(k):
        parent.setdefault(k, k)
        while parent[k] != k:
            parent[k] = parent[parent[k]]; k = parent[k]
        return k
    def union(a, b): parent[find(a)] = find(b)
    for lay, net, w, pts in tracks:
        for a, b in zip(pts, pts[1:]): union((lay, a), (lay, b))
        for pt in pts:
            for r, p, x, y, dia, _, n in pads:
                if math.hypot(pt[0] - x, pt[1] - y) < 0.01:
                    if n != net: errs.append(f"track {net} ends on pad {r}.{p} of net {n}")
                    union((lay, pt), ("pad", r, p))
            for lay2, net2, _, a, b in segs(lay):           # T-junctions on the same layer
                if net2 == net and d_pt_seg(pt, a, b) < 0.01: union((lay, pt), (lay2, a))
    for lay, net, w, a, b in segs():                        # pads sitting mid-track
        for r, p, x, y, dia, _, n in pads:
            if n == net and d_pt_seg((x, y), a, b) < 0.01: union((lay, a), ("pad", r, p))
    for net, members in NETLIST.items():
        roots = {find(("pad", r, p)) for r, p in members}
        if len(roots) != 1: errs.append(f"net {net} is not fully connected ({len(roots)} islands)")
    return errs

# ---------------------------------------------------------------- silkscreen
G = {  # stroke font on a 4 x 6 grid, y up
 "0": [[(0,0),(4,0),(4,6),(0,6),(0,0)]], "1": [[(1,5),(2,6),(2,0)]],
 "2": [[(0,6),(4,6),(4,3),(0,3),(0,0),(4,0)]], "3": [[(0,6),(4,6),(4,0),(0,0)],[(1,3),(4,3)]],
 "4": [[(0,6),(0,3),(4,3)],[(4,6),(4,0)]], "5": [[(4,6),(0,6),(0,3),(4,3),(4,0),(0,0)]],
 "6": [[(4,6),(0,6),(0,0),(4,0),(4,3),(0,3)]], "7": [[(0,6),(4,6),(4,0)]],
 "8": [[(0,0),(4,0),(4,6),(0,6),(0,0)],[(0,3),(4,3)]], "9": [[(4,3),(0,3),(0,6),(4,6),(4,0),(0,0)]],
 "A": [[(0,0),(0,4),(2,6),(4,4),(4,0)],[(0,3),(4,3)]],
 "B": [[(0,0),(0,6),(3,6),(4,5),(4,4),(3,3),(0,3)],[(3,3),(4,2),(4,1),(3,0),(0,0)]],
 "C": [[(4,6),(0,6),(0,0),(4,0)]], "D": [[(0,0),(0,6),(3,6),(4,5),(4,1),(3,0),(0,0)]],
 "E": [[(4,6),(0,6),(0,0),(4,0)],[(0,3),(3,3)]], "F": [[(0,0),(0,6),(4,6)],[(0,3),(3,3)]],
 "G": [[(4,6),(0,6),(0,0),(4,0),(4,3),(2,3)]], "H": [[(0,0),(0,6)],[(4,0),(4,6)],[(0,3),(4,3)]],
 "I": [[(2,0),(2,6)]], "J": [[(4,6),(4,0),(0,0),(0,2)]], "K": [[(0,0),(0,6)],[(4,6),(0,3),(4,0)]],
 "L": [[(0,6),(0,0),(4,0)]], "N": [[(0,0),(0,6),(4,0),(4,6)]], "O": [[(0,0),(4,0),(4,6),(0,6),(0,0)]],
 "R": [[(0,0),(0,6),(4,6),(4,3),(0,3)],[(2,3),(4,0)]], "S": [[(4,6),(0,6),(0,3),(4,3),(4,0),(0,0)]],
 "T": [[(0,6),(4,6)],[(2,6),(2,0)]], "U": [[(0,6),(0,0),(4,0),(4,6)]], "V": [[(0,6),(2,0),(4,6)]],
 "+": [[(0,3),(4,3)],[(2,1),(2,5)]], "-": [[(0,3),(4,3)]], " ": [],
}
silk = []   # list of ((x1,y1),(x2,y2))
def text(s, cx, cy, h=1.0):
    u = h / 6.0; adv = 5.6 * u
    x0 = cx - (len(s) * adv - 1.6 * u) / 2
    for i, ch in enumerate(s):
        for line in G[ch]:
            pts = [(x0 + i * adv + gx * u, cy + h / 2 - gy * u) for gx, gy in line]
            silk.extend(zip(pts, pts[1:]))
def poly(pts, close=True):
    pts = list(pts) + ([pts[0]] if close else [])
    silk.extend(zip(pts, pts[1:]))
def rect(x1, y1, x2, y2): poly([(x1, y1), (x2, y1), (x2, y2), (x1, y2)])
def circle(cx, cy, r, n=48): poly([(cx + r * math.cos(2 * math.pi * i / n), cy + r * math.sin(2 * math.pi * i / n)) for i in range(n)])

def build_silk():
    rect(1.7, 2.75, 4.3, 23.05)                                   # J1 outline
    rect(15.3, 3.9, 20.32, 21.34)                                 # U1 body, between the pin rows
    poly([(16.9, 3.9), (16.9, 4.7), (18.7, 4.7), (18.7, 3.9)], close=False)   # pin-1 end notch
    circle(12.45, 5.0, 0.25, 12)                                  # pin-1 dot
    text("U1", 17.8, 9.6, 1.0); text("AHCT", 17.8, 14.2, 0.9); text("125", 17.8, 16.0, 0.9)
    rect(6.9, 9.5, 9.1, 15.8);   text("10K", 8.0, 19.75, 0.9)     # R2 body
    rect(10.1, 12.0, 12.3, 18.3); text("470", 11.2, 8.25, 0.9)    # R1 body
    rect(29.2, 5.25, 36.7, 5.9);  text("F1", 33.0, 7.0, 0.9)      # F1 marker bar
    circle(32.96, 13.0, 5.2);     text("+", 38.9, 13.0, 1.2)      # C1 body and polarity
    text("1000U", 32.96, 16.3, 0.8)
    rect(26.4, 3.9, 28.6, 8.65)                                   # C2 outline
    rect(29.1, 21.2, 36.8, 23.8)                                  # J2 outline
    text("G", 30.42, 20.2, 0.9); text("DI", 32.96, 20.2, 0.9); text("5V", 35.5, 20.2, 0.9)
    text("RING", 38.9, 22.5, 0.9)
    text("HALO DRIVER V1", 20.5, 26.4, 1.0)

def build_j1_labels():
    # header pin names sit on the BACK of the board (the front has traces and R2 there)
    return [("5V", 0), ("GND", 1), ("3V3", 2), ("44", 3), ("43", 4), ("17", 5), ("18", 6), ("16", 7)]

# ---------------------------------------------------------------- gerber / drill output
def fmt(v): return str(int(round(v * 1e6)))
def gy(y): return H - y        # gerber y points up

def write_gerber(ext, func, draws, flashes):
    ap, body = {}, []
    def code(d):
        if d not in ap: ap[d] = 10 + len(ap)
        return ap[d]
    last = None
    for w, a, b in draws:
        c = code(round(w, 3))
        if c != last: body.append(f"D{c}*"); last = c
        body.append(f"X{fmt(a[0])}Y{fmt(gy(a[1]))}D02*")
        body.append(f"X{fmt(b[0])}Y{fmt(gy(b[1]))}D01*")
    for d, x, y in flashes:
        c = code(round(d, 3))
        if c != last: body.append(f"D{c}*"); last = c
        body.append(f"X{fmt(x)}Y{fmt(gy(y))}D03*")
    head = ["G04 Halo driver v1, generated by gen_pcb.py*", f"%TF.FileFunction,{func}*%",
            "%FSLAX46Y46*%", "%MOMM*%", "%LPD*%", "G01*"]
    head += [f"%ADD{c}C,{d:.3f}*%" for d, c in ap.items()]
    with open(os.path.join(OUT, f"{NAME}.{ext}"), "w") as f:
        f.write("\n".join(head + body + ["M02*"]) + "\n")

def write_drill(fname, items):
    sizes = sorted({d for _, _, d in items})
    L = ["M48", "METRIC"] + [f"T{i + 1}C{d:.3f}" for i, d in enumerate(sizes)] + ["%", "G90", "G05"]
    for i, d in enumerate(sizes):
        L.append(f"T{i + 1}")
        L += [f"X{x:.3f}Y{gy(y):.3f}" for x, y, dd in items if dd == d]
    with open(os.path.join(OUT, fname), "w") as f:
        f.write("\n".join(L + ["M30"]) + "\n")

def write_svg(back_silk):
    col = {T: "#c8442a", B: "#2a62c8"}
    s = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="-2 -10 {W + 4} {W + 4}" width="1056" height="1056">',
         f'<rect x="-2" y="-10" width="{W + 4}" height="{W + 4}" fill="#fff"/>',
         f'<rect x="0" y="0" width="{W}" height="{H}" fill="#0d3b24" stroke="#000" stroke-width="0.1"/>']
    for lay in (B, T):
        for _, net, w, a, b in segs(lay):
            s.append(f'<line x1="{a[0]}" y1="{a[1]}" x2="{b[0]}" y2="{b[1]}" stroke="{col[lay]}" stroke-width="{w}" stroke-linecap="round" opacity="0.85"/>')
    for a, b in silk:
        s.append(f'<line x1="{a[0]:.3f}" y1="{a[1]:.3f}" x2="{b[0]:.3f}" y2="{b[1]:.3f}" stroke="#fff" stroke-width="0.15" stroke-linecap="round"/>')
    for r, p, x, y, dia, dr, n in pads:
        s.append(f'<circle cx="{x}" cy="{y}" r="{dia / 2}" fill="#d8b24a"/><circle cx="{x}" cy="{y}" r="{dr / 2}" fill="#111"/>')
    for x, y, d in holes:
        s.append(f'<circle cx="{x}" cy="{y}" r="{d / 2}" fill="#fff" stroke="#000" stroke-width="0.1"/>')
    s.append("</svg>")
    with open(os.path.join(os.path.dirname(OUT), "preview.svg"), "w") as f: f.write("\n".join(s))

def main():
    errs = check()
    if errs:
        print("CHECKS FAILED:"); [print("  -", e) for e in errs]; sys.exit(1)
    os.makedirs(OUT, exist_ok=True)
    build_silk()
    front_silk = list(silk); silk.clear()
    for lab, i in build_j1_labels():                 # back silk, mirrored so it reads from the back
        text(lab, 7.4, 4.0 + i * P, 0.9)
    text("TO BOARD HEADER", 22.0, 26.4, 0.9)
    back = [((2 * 7.4 - a[0], a[1]), (2 * 7.4 - b[0], b[1])) if a[1] < 25 else ((2 * 22.0 - a[0], a[1]), (2 * 22.0 - b[0], b[1])) for a, b in silk]
    silk.clear(); silk.extend(front_silk)
    for lay, ext, fn in ((T, "GTL", "Copper,L1,Top"), (B, "GBL", "Copper,L2,Bot")):
        write_gerber(ext, fn, [(w, a, b) for _, _, w, a, b in segs(lay)], [(d, x, y) for _, _, x, y, d, _, _ in pads])
    mask = [(d + 0.1, x, y) for _, _, x, y, d, _, _ in pads] + [(d + 0.2, x, y) for x, y, d in holes]
    write_gerber("GTS", "Soldermask,Top", [], mask)
    write_gerber("GBS", "Soldermask,Bot", [], mask)
    write_gerber("GTO", "Legend,Top", [(0.15, a, b) for a, b in front_silk], [])
    write_gerber("GBO", "Legend,Bot", [(0.15, a, b) for a, b in back], [])
    c = [(0, 0), (W, 0), (W, H), (0, H), (0, 0)]
    write_gerber("GKO", "Profile,NP", [(0.1, a, b) for a, b in zip(c, c[1:])], [])
    write_drill(f"{NAME}-PTH.DRL", [(x, y, dr) for _, _, x, y, _, dr, _ in pads])
    write_drill(f"{NAME}-NPTH.DRL", holes)
    write_svg(back)
    z = os.path.join(os.path.dirname(OUT), f"{NAME}-gerbers.zip")
    with zipfile.ZipFile(z, "w", zipfile.ZIP_DEFLATED) as zf:
        for fn in sorted(os.listdir(OUT)): zf.write(os.path.join(OUT, fn), fn)
    nseg = sum(1 for _ in segs())
    print(f"OK: {len(pads)} pads, {nseg} track segments, {len(NETLIST)} nets, checks passed. Files in {OUT}")

if __name__ == "__main__":
    main()
