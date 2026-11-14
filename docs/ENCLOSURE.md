# Enclosure

**Status: draft, written early (2026-11-13) at the owner's request.** Phase 4 has not started, and Phase 2 is still waiting for approval. Nothing here is decided until the owner approves it. The proposed decision is D-029 in [DECISIONS.md](DECISIONS.md).

Goals set by the owner:
1. A **foldable stand**.
2. **Every part fits neatly**, both the required parts and the optional ones.
3. The case should **last about 30 years**.

---

## 1. What "30 years" means here

A printed case can last 30 years indoors. Some of the parts inside it cannot. A Li-ion cell lasts about 5–10 years, an AMOLED fades over time, and a USB socket wears out with use. So the target is:

> **The case lasts 30 years. Every part that wears out inside it can be replaced with one screwdriver, with nothing glued and nothing printed again.**

How this draft gets there:

| Rule | How |
|------|-----|
| No glue, no adhesive tape, no snap-fits | Snap-fits fatigue and adhesive dries out within about 10 years. Everything is held by screws. |
| Never thread a screw straight into plastic | Each screw goes into a **brass heat-set insert**, which survives hundreds of removals. Plastic threads strip after about 10. |
| One screw type, one tool | M3 stainless (A2) socket-head screws throughout, 2.5 mm hex key. The display board keeps its own M2 holes. |
| Metal at every moving or wearing point | Stainless steel hinge pin turning in brass bushings. Printed plastic never rubs on printed plastic. |
| Wear lands on cheap parts | The USB-C cable plugs into a $3 screw-mounted panel socket, never into the board's own port (§5). |
| Heat-resistant plastic | ASA or PETG, never PLA. PLA softens at about 55 °C and creeps under steady load. The ring and fuse may reach 60 °C (POWER.md M6). |
| Parts fail independently | The diffuser, stand, stop block, back cover and board carrier are separate parts. A scratched diffuser or a cracked stand is one remake. |
| Next display, same case | The board sits on a printed **board carrier**. If Waveshare stops making this board, only the carrier is redrawn for the replacement. |
| Files that still open in 30 years | Parametric source in plain-text OpenSCAD, plus STEP and STL exports, in `/enclosure`. The print settings and the full screw list are written down in this file. |
| Spares | Make a second diffuser and stand, and keep 10 spare screws and 4 spare inserts in a bag with the clock. |

## 2. Stand options compared (revised 2026-11-13)

Owner's brief for the revision: tools are a **3D printer and a laser cutter**; small bought metal parts are fine; the stand **folds or pops out**; the clock **leans back** (top away from the sleeper, screen facing slightly up) for a nightstand below eye level.

All options keep every electronic part in the head, so no wire crosses a hinge.

| | A. Thin kickstand leg | B. Head in a U-shaped base | C. Weighted foot plate under the head (first draft) | **D. Pop-out back panel** |
|---|---|---|---|---|
| How it works | A narrow leg hinged on the back | Head turns on two side pins | A plate hinged at the bottom-rear edge | The lower half of the back swings out and props the head |
| Leans back | Yes | Yes | Yes | **Yes, 15°** |
| Steady under a tap | Fair: narrow, can twist | Good | Good | **Good:** 60 mm wide toe, hard stop, weight low in the head |
| Folded | 34 mm, leg visible | About 75 mm | About 40 mm, plate on the back | **About 38 mm, flush, nothing sticks out** |
| Parts | Low | High | Low | **Low:** one panel, one pin, one stop block |
| Weak points over 30 years | Thin leg | Two pivots | One hinge | One hinge, 60 mm long, metal pin |
| Score (my judgement, out of 5) | 2.5 | 3 | 4 | **4.5** |

**Chosen: D (owner approved 2026-11-13).** It is the same idea as C, moved from under the head to the back of it. The clock then rests on its own bottom edge and the stand only props it, so it folds flush.

## 3. Layout (option D)

```
 in use, side view (front on the left)        back view

        ┌────────┐                              ╭────────╮
       ╱        ╱                              │ speaker │
      ╱  head  ╱ ○  hinge, mid-height          │ ─────── │  ← hinge line
     ╱        ╱   ╲                            │▒▒▒▒▒▒▒▒▒│
    ╱        ╱     ╲  stand, 40 mm             │▒ stand ▒│
   └────────┘       ╲                           ╰▒▒▒▒▒▒▒╯
 ══▀▀═══════════════▀══  table
   pads              toe
```

### Head, front to back (sizes to be confirmed by measuring the board)

| Layer | Depth (mm) | Contents |
|-------|-----------:|----------|
| Front bezel and halo diffuser | 3 | **Laser-cut** opal acrylic ring (§4). The opening leaves the 44.5 mm active area clear. The bezel must not press on the touch glass (pressure can cause phantom touches); a 0.5 mm gap keeps it clear. |
| Display board | 10.4 | Sits in the board carrier. The ring surrounds it: board about 49 mm across, ring opening 52.2 mm. |
| Light gap and ring | 8 + 3.3 | The ring sits 8 mm behind the diffuser so 24 LED points blend into one even glow. A white inner wall reflects light forward. A thin wall stops light leaking onto the screen edge. |
| Chassis plate | 2.5 | Brass inserts for everything. Cable holes have rounded edges. |
| Rear bay | 12 | Battery 50 × 34 × 10 mm in a tray with 2 mm of swell room, so it never presses toward the screen. Speaker 30 × 20 × 6.8 mm above it, behind the grille. The fuse, capacitor and resistor go in the curved space at the top. A stack of steel washers (about 100 g) sits at the very bottom. |
| Back cover with stand recess | 2.5 + 5 | Upper half: speaker grille and vents. Lower half: a 5 mm deep recess that the stand folds into. |
| **Head total** | **about 38 mm deep, about 88 mm across** | |

The bottom of the head is not round. It has a flat facet cut at 15°, about 45 mm wide, with two silicone pads, so the leaning head sits flat on the table and cannot roll.

### Stand

- **Shape:** the lower half of the back, a half-disc about 60 mm wide at the toe and 40 mm from hinge to toe, 5 mm thick. Printed in ASA with the hinge knuckles as part of it. A silicone strip on the toe.
- **Hinge:** two short M3 stainless shoulder screws, one from each side, turning in brass tube (4 mm outside, 3 mm bore) pressed into the knuckles. Each bore is under 25 mm deep, so it can be drilled in metal as well as printed (§12). A dab of thread-lock holds each screw; a wave washer on one side sets the friction.
- **Stop block:** a small separate printed block, held by one screw, that the stand's knuckle lands on when open. The stop, not friction, carries the load. The stand opens about 50° to give a 15° lean. To change the lean, print another block (10°, 15° and 20° versions).
- **Pop out:** a finger notch at the toe. A 6 × 2 mm magnet pair holds it shut. Pull with a fingernail and it swings to the stop.
- **Print direction:** knuckles printed so the load runs along the layers.

### Stability (calculated, not yet tested)

| Figure | Value |
|--------|-------|
| Weight, with about 100 g of steel washers low in the head | about 280 g (estimate) |
| Support: front pads to stand toe | about 70 mm |
| Centre of mass behind the front pads | about 28 mm |
| Push at the top of the screen that tips it backward | about 1.5 N (about 150 g of finger force) |
| Push that makes it slide (silicone on wood) | about 2 N |

A normal tap on a touchscreen is under 1 N, so the margin is there but not large. If check C1 fails, the fixes are more washers or a 50 mm stand. Without the washers the tip force drops to about 1 N, so the weight is recommended.

### What the laser cutter makes

| Part | Material | Why laser instead of print |
|------|----------|----------------------------|
| Halo diffuser ring | 3 mm **cast opal acrylic** | Perfectly even glow with no layer lines. Acrylic does not yellow indoors for decades. |
| Speaker gasket, toe strip, foot pads | 1.5 mm silicone sheet | Replaces TPU prints and foam. |
| Optional: face for the stand or back | 3 mm acrylic or plywood veneer | Looks only. |

Do not laser-cut polycarbonate or PVC: polycarbonate chars and PVC gives off chlorine gas.

## 4. Light, sound, heat

- **Halo diffuser:** laser-cut 3 mm cast opal acrylic ring, a separate part held by the bezel. Fallback: natural PETG printed flat.
- **Speaker:** the unit is sealed, so it needs no tuned chamber, only a grille (HARDWARE.md §4). Use printed holes, not cloth, since cloth fades and collects dust. Seal it with a laser-cut silicone gasket rather than foam, because foam crumbles within about 10 years.
- **Heat:** the ring at the 0.9 A cap gives off about 4.5 W during a sunrise. Intake slots along the bottom edge and exhaust slots on top let air flow up through the light gap past the ring. The battery sits behind the chassis plate, away from the ring: every 10 °C cooler roughly doubles battery life. Bench test M6 is repeated with the case closed.
- **Capacitor:** 105 °C-rated (EEU-FR is). Kept away from the ring, it lasts well beyond 30 years.

## 5. Ports and buttons

| Item | Plan | Why |
|------|------|-----|
| Power in | Screw-mounted USB-C panel socket (pigtail) in the back cover. It feeds the board's USB-C inside, or, with PCB v2, the PCB's own socket (D-021). | A USB-C socket is rated for about 10 000 plug-ins. Wear lands on a $3 part held by two screws. **Optional:** without it, a hole gives access to the board's port directly. |
| Board's own USB-C (flashing) | Reachable through a side hole with a TPU plug | Firmware updates until OTA arrives (Phase 7). |
| BOOT button (fallback dismiss, D-007) | Printed plunger with a TPU spring tab, flush with the case, at the rear edge where a hand finds it in the dark | It must work even if the touch screen fails. |
| PWR button | Recessed plunger. It needs a pen tip or fingernail. | R27: no accidental power-off. |
| TF card | Reached by removing the back cover | Rarely used. |

## 6. Optional parts, each with a reserved place (D-025)

| Part | Where | If it is not fitted |
|------|-------|---------------------|
| Battery | Rear tray | The tray stays empty. Firmware runs from USB (R36). |
| Fuse, 1000 µF capacitor, 470 Ω resistor (inline) | Top curve of the rear bay, held by a printed clip | — (needed above 0.3 A) |
| Level shifter 74AHCT125 | Same space, on a small perfboard | — |
| Halo driver PCB v2 | Redrawn to the top-curve shape (about 60 × 14 mm usable) once the board is measured (D-020) | Inline parts are used instead. |
| USB-C panel socket | Back cover | A plain hole reaches the board's port. |
| Steel washers | Bottom of the rear bay | The clock still stands, but tips at about 1 N instead of 1.5 N. |

## 7. Materials and print settings (draft)

| Part | Material | Settings |
|------|----------|----------|
| Head shell, chassis plate, back cover, stand, stop block | **ASA** (best heat and UV resistance) or PETG | 0.2 mm layers, 4 walls, 40 % gyroid infill. 5 walls around each insert. |
| Halo diffuser | 3 mm cast opal acrylic, laser-cut | Fallback: natural PETG, 100 % infill, printed flat. |
| Gasket, toe strip, pads | 1.5 mm silicone sheet, laser-cut | — |
| Button springs, port plugs | TPU 95A (or PETG flexures if no TPU) | — |

Inserts: M3 × 5.7 mm brass heat-set (CNC Kitten or Ruthex style). Hole size as the insert maker specifies, usually 4.0–4.2 mm.

## 8. Shopping list for the case (approximate)

| Item | Qty | ~Cost |
|------|-----|-------|
| M3 brass heat-set inserts | 20 (10 spare) | $5 |
| M3 × 8 stainless socket-head screws | 20 (10 spare) | $4 |
| M3 stainless shoulder screws, 4 mm shoulder × 20 mm, plus one wave washer (hinge) | 2 | $4 |
| Brass tube, 4 mm outside, 3 mm bore, 100 mm | 1 | $3 |
| Steel washers, about 100 g in total | — | $3 |
| Cast opal acrylic, 3 mm, 100 × 100 mm | 1 | $4 |
| Silicone sheet, 1.5 mm, 100 × 100 mm | 1 | $4 |
| 6 × 2 mm neodymium magnets | 2 | $1 |
| USB-C panel-mount extension, screw flange (optional) | 1 | $4 |
| ASA or PETG filament, natural PETG, TPU | — | owner's stock |

## 9. Measurements needed before CAD (Phase 4, step 1)

Board size from a third-party listing (Core Electronics): **48.96 mm across × 10.40 mm deep, 3 × M2 holes**. Not confirmed against Waveshare's own drawing.

| # | Measure | Why |
|---|---------|-----|
| E1 | Board diameter, depth, and the position of the M2 holes (calipers) | Board carrier |
| E2 | Positions of the USB-C port, BOOT and PWR buttons, TF slot, battery and speaker connectors, and the 8-pin header (V9) | Port holes and plungers |
| E3 | The battery as bought, with its wires and plug | Tray size |
| E4 | Speaker mounting flange | Gasket |
| E5 | Front of the glass to the active area, and the bezel width | Front opening |
| E6 | Ring pad positions (DIN, 5V, GND) | Wire exits |

## 10. Acceptance checks (to become T-33 in the test plan)

| # | Check | Pass |
|---|-------|------|
| C1 | Tap and 2 s hold at the top edge of the screen; then push there with a kitchen scale | The clock does not slide or tip in use. It tips at no less than 1.5 N (150 g). |
| C2 | Fold and unfold 500 times (about 15 years of weekly travel) | The stand still lands on its stop and stays shut when folded. No cracks. |
| C3 | Sunrise at the cap for 30 min, case closed | Ring < 60 °C, battery < 40 °C (M6) |
| C4 | Halo evenness | No visible LED dots from 1 m |
| C5 | Take out the battery, board and speaker, then put them back | 2.5 mm hex key only. Under 10 minutes. Nothing broken. |
| C6 | Drop folded from 75 cm onto carpet | Still works. Any damage is to a reprintable part only. |
| C7 | BOOT plunger in the dark, by feel | Found and pressed within 5 s |
| C8 | Metal near the screen: wrap the plastic prototype's shell in aluminium foil (not the front), tied to ground and then not tied, and run the touch tests | No false or missed touches either way. A cheap early warning before paying for metal. |

## 11. Open questions for the owner

1. ~~Option D (pop-out back panel, 15° lean back): OK?~~ **Approved 2026-11-13.**
5. What can your laser cutter cut: acrylic, plywood, silicone sheet? What is its bed size and power?
2. Halo on the **front** around the screen (this draft), or facing **backward** to wash the wall behind? A rear glow is softer but needs a wall close behind.
3. Colour and finish: one colour, or a contrasting stand and bezel?
4. Can you measure E1–E6 with calipers? If not, I work from Waveshare's drawing and you test-fit a quick print.

## 12. Metal-ready rules (D-043)

The owner wants the final case in **metal**, made by CNC machining or metal 3D printing. The plastic print is the prototype. So every part is drawn once and must be makeable three ways: plastic print, CNC, metal print. The CAD has one switch, `material = plastic | metal`, that changes only the items in the last table below.

### Shape rules that all three methods accept

| Rule | Value | Reason |
|------|-------|--------|
| Each part is cut from two opposite sides only | front and back | Two CNC set-ups, no special fixtures. The head is round, so the shell can be turned on a lathe first. |
| No undercuts, no hidden pockets, no enclosed voids | — | A cutter cannot reach them; a metal print traps powder in them. |
| Inside corners of pockets | radius ≥ 2 mm (3 mm where it fits) | A 4 mm cutter leaves a 2 mm radius. Square inside corners cannot be milled. Parts that sit in pockets (battery tray, speaker) get matching rounded corners or corner reliefs. |
| Pocket depth | ≤ 4 × the cutter width; the 12 mm rear bay uses a 4–6 mm cutter | Long thin cutters chatter and break. |
| Walls | 2.5 mm everywhere, 1.5 mm minimum | Fine for ASA, aluminium and printed metal alike. |
| Holes | standard drill sizes; depth ≤ 8 × diameter | Hence two short hinge bores, not one 70 mm bore. |
| Grille and vent holes | round, 1.5–2 mm, straight through | Drilled in metal, printed in plastic. No fine slots. |
| Outside edges | 0.5 mm chamfer or 1 mm radius | Comfort, and anodising is thin on sharp edges. |
| Overhangs | ≥ 45° from flat, or a flat face that can be machined after | Needed for both plastic and metal printing. |
| No snap-fits, no living hinges, no press-fit of printed plastic | — | Metal does not flex. Already ruled out for long life (§1). |
| Text and marks | engraved 0.3 mm deep, strokes ≥ 0.5 mm | Works by cutter, laser or print. |

### What a metal case changes

| Topic | Effect | What the design does |
|-------|--------|----------------------|
| Radio | None. The radios are never used (D-030), so a closed metal shell costs nothing. | — |
| Short circuits | A metal shell conducts. Anodising insulates but scratches through. | No circuit board, ring or battery ever touches the shell. The **board carrier, ring seat and battery tray stay plastic** in every version. 1.5 mm air gap or a plastic wall between any solder joint and metal. |
| Touch screen | A metal bezel close to a capacitive screen can cause false or missed touches, more so if the metal is not grounded. | The acrylic diffuser ring already keeps metal about 9 mm from the glass edge. One screw ties the shell to the circuit ground. **Check C8.** |
| Heat | Better. Aluminium spreads the ring's 4.5 W, so the vents matter less. | The ring seat is thin (1 mm plastic) so heat reaches the shell. The shell may feel warm during a sunrise; C3 also measures the outside (under 45 °C). |
| Weight | Better. An aluminium shell weighs about 150 g more than ASA. | The steel washers become optional. The stability figures in §3 are redone for metal. |
| Sound | A stiff shell rattles less, but hard contact buzzes. | The silicone speaker gasket stays. The stand closes onto a silicone strip, never metal on metal. |
| Light | Bare metal inside the light gap tints the glow. | The plastic ring seat is white and is the reflector in every version. |
| Battery | A swelling cell against metal is a hazard. | Plastic tray with 2 mm swell room (already there). No sharp metal edge faces the cell. |
| Corrosion | Stainless screws and brass in aluminium are fine indoors. | Anodised or bead-blasted finish. No bare steel. |

### Items the `material` switch changes

| Item | Plastic prototype | Metal |
|------|-------------------|-------|
| Screw threads | Brass heat-set inserts, 4.0 mm holes | M3 tapped straight into the metal, 6 mm deep, 2.5 mm holes. The four back-cover threads get stainless thread inserts (Helicoil), since they are opened most often. |
| Sliding fit (stand in recess, pins, plungers) | 0.25 mm gap | 0.10 mm gap |
| Hinge bore | Printed, brass tube pressed in | Drilled and reamed to 4.0 mm, same brass tube |
| Stop block | Separate printed block | Separate block, machined or printed metal. It stays separate so the lean angle can still be changed. |
| Magnet seat | Pocket with a screw-on cap | Same. Aluminium is not magnetic, so both magnets stay; the steel washers must sit away from them. |
| Shell | ASA or PETG | **Aluminium 6061-T6, anodised** (recommended: light, cheap to machine, stable for decades). Brass or stainless cost about 3× more to machine and weigh 3× as much. |

### Extra rules for metal 3D printing (aluminium AlSi10Mg or 316L steel)

- Holes come out small and rough: every threaded or fitted hole is printed 0.5 mm undersize, then drilled and tapped.
- Faces that meet another part (back cover seat, hinge cheeks, diffuser seat) get 0.5 mm extra for a finishing cut.
- The surface is grainy; bead-blast it. Fine detail under 0.5 mm is lost.
- Order from a print service: the part shrinks and warps slightly, so tight fits are always machined after.

**CNC or metal print?** For this shape, CNC in aluminium is the better fit: it is a simple round body with two open sides, and it gives clean threads and faces straight off the machine. Metal printing only wins if the shape gains details that cannot be milled, which these rules forbid anyway.
