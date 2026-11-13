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
| Parts fail independently | The diffuser, foot, back cover and board carrier are separate prints. A yellowed diffuser or a cracked foot is one reprint. |
| Next display, same case | The board sits on a printed **board carrier**. If Waveshare stops making this board, only the carrier is redrawn for the replacement. |
| Files that still open in 30 years | Parametric source in plain-text OpenSCAD, plus STEP and STL exports, in `/enclosure`. The print settings and the full screw list are written down in this file. |
| Spares | Print a second diffuser and foot, and keep 10 spare screws and 4 spare inserts in a bag with the clock. |

## 2. Stand options compared

All three options keep every electronic part in the head, so no wire crosses a hinge and nothing flexes over the years.

| | A. Kickstand leg | B. Head in a U-shaped base (yoke) | **C. Fold-out weighted foot** |
|---|---|---|---|
| How it works | A thin leg hinged on the back, like a picture frame | The head turns on two side pins inside a U-shaped base | A flat plate hinged at the head's bottom-rear edge swings out behind the head |
| Steady when you tap or hold the screen | Fair. Light, and the leg can slide | Good | **Good.** The weight sits low and behind, and the hinge stop takes the push |
| Weighted, non-slip base (R33) | No room for a weight | Yes | **Yes, a steel plate in the foot** |
| Folded size | About 34 mm thick | About 75 mm. A pin at the centre leaves the folded head high above the base | **About 40 mm thick, one round puck** |
| Tilt | 2 fixed angles | Any angle | 3 detents: 0°, 12°, 24° back |
| Number of parts | Lowest | Highest: base, 2 arms, 2 pivots | Low: one plate, one pin, a weight |
| Weak points over 30 years | The thin leg and its small hinge | Two pivots, more to loosen | One hinge, short and thick, made of metal |
| Score (my judgement, out of 5) | 2.5 | 3.5 | **4.5** |

**Recommended: C.** It meets R33, is the most stable under a finger and folds the smallest. Its one hinge is short, thick and metal.

## 3. Layout (option C)

```
 side section (front on the left)

   ┌──────────┐
   │▓ ░░░░ ▒▒ │   ▓ halo diffuser (front ring around the screen)
   │  ████ ▒▒ │   █ display board, about 49 mm, on 3 × M2 screws to the board carrier
   │▓ ████│▒▒ │   ░ light gap, 8 mm, between the ring LEDs and the diffuser
   │  ████│▒▒ │   │ chassis plate with brass inserts
   │  ████│▒▒ │   ▒ battery tray (rear), speaker beside it, grille in the back cover
   │▓ ░░░░│▒▒ │
   └──────────┘○──────────────┐   ○ stainless pin, brass bushings, detent stop
          ▔▔    └──[ steel ]───┘   fold-out foot, 60 mm, with silicone feet
```

### Head, front to back (sizes to be confirmed by measuring the board)

| Layer | Depth (mm) | Contents |
|-------|-----------:|----------|
| Front bezel and halo diffuser | 2.5 | The opening leaves the 44.5 mm active area clear. The bezel must not press on the touch glass (pressure can cause phantom touches); a 0.5 mm gap or a thin TPU ring keeps it clear. |
| Display board | 10.4 | Sits in the board carrier. The ring surrounds it: board about 49 mm across, ring opening 52.2 mm. |
| Light gap and ring | 8 + 3.3 | The ring sits 8 mm behind the diffuser so 24 LED points blend into one even glow. A white inner wall reflects light forward. A thin wall stops light leaking onto the screen edge. |
| Chassis plate | 2.5 | Brass inserts for everything. Cable holes have rounded edges. |
| Rear bay | 12 | Battery 50 × 34 × 10 mm in a tray with 2 mm of swell room on top, so it never presses toward the screen. Speaker 30 × 20 × 6.8 mm beside it. The fuse, capacitor and resistor go in the curved space at the top. |
| Back cover | 2.5 | Speaker grille holes, vents, USB-C panel socket, hinge knuckles. |
| **Head total** | **about 32 mm deep, about 88 mm across** | |

Fit check: the battery and speaker side by side make a 54 × 50 mm block. Its diagonal is 74 mm, which fits the 83 mm inner circle.

### Foot

- About 88 × 60 × 7 mm, ASA, with a 3 mm steel plate (about 80 g) held by two screws. The total weight of about 250 g keeps it from moving when the screen is tapped.
- Four press-in silicone bumpers go into holes, with no adhesive. When they harden in 10–20 years, pull them out and press in new ones.
- A cable notch in the foot leads the USB-C cable straight back.
- **Folded:** the foot swings up flat against the back cover, and a small magnet pair keeps it shut in a bag. Neodymium magnets keep their strength for decades at room temperature. The whole clock becomes a puck about 88 mm across and about 40 mm thick.

### Hinge

- Stainless steel shoulder screw (M3, 4 mm shoulder) through two brass flanged bushings pressed into the printed knuckles.
- A wave washer and a nylon-insert locknut set the friction. When it loosens after years of use, a quarter turn restores it.
- **Detents:** a printed cam with three notches (0°, 12°, 24°) and a hard stop. The stop, not the friction, takes the push from a tap or hold on the screen.
- Print the knuckles so the load runs along the layers, never across them, so the layers cannot split.

## 4. Light, sound, heat

- **Halo diffuser:** natural (uncoloured) PETG or opal polycarbonate, 1.6–2 mm thick, separate part. Polycarbonate yellows under sunlight but is fine at a bedside. Either way it is a cheap reprint.
- **Speaker:** the unit is sealed, so it needs no tuned chamber, only a grille (HARDWARE.md §4). Use printed holes, not cloth, since cloth fades and collects dust. Seal it with a printed TPU gasket rather than foam, because foam crumbles within about 10 years.
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
| Steel weight | Foot | The foot still works, just lighter. |

## 7. Materials and print settings (draft)

| Part | Material | Settings |
|------|----------|----------|
| Head shell, chassis plate, back cover, foot | **ASA** (best heat and UV resistance) or PETG | 0.2 mm layers, 4 walls, 40 % gyroid infill. 5 walls around each insert. |
| Halo diffuser | Natural PETG | 100 % infill, printed flat. Test the evenness before committing. |
| Gaskets, button springs, port plugs | TPU 95A | — |

Inserts: M3 × 5.7 mm brass heat-set (CNC Kitten or Ruthex style). Hole size as the insert maker specifies, usually 4.0–4.2 mm.

## 8. Shopping list for the case (approximate)

| Item | Qty | ~Cost |
|------|-----|-------|
| M3 brass heat-set inserts | 20 (10 spare) | $5 |
| M3 × 8 stainless socket-head screws | 20 (10 spare) | $4 |
| M3 stainless shoulder screw, 4 mm shoulder, plus nylon-insert locknut and wave washer | 1 set | $3 |
| Brass flanged bushings, 4 mm bore | 2 | $2 |
| Steel plate, about 80 × 50 × 3 mm (or a stack of large washers) | 1 | $3 |
| Press-in silicone bumpers | 6 (2 spare) | $3 |
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
| C1 | Tap and 2 s hold at the screen edge, at each tilt | The clock does not slide or tip. The hinge does not move. |
| C2 | Fold and unfold 500 times (about 15 years of weekly travel) | The hinge holds each detent. No cracks. |
| C3 | Sunrise at the cap for 30 min, case closed | Ring < 60 °C, battery < 40 °C (M6) |
| C4 | Halo evenness | No visible LED dots from 1 m |
| C5 | Take out the battery, board and speaker, then put them back | 2.5 mm hex key only. Under 10 minutes. Nothing broken. |
| C6 | Drop folded from 75 cm onto carpet | Still works. Any damage is to a reprintable part only. |
| C7 | BOOT plunger in the dark, by feel | Found and pressed within 5 s |

## 11. Open questions for the owner

1. Option C (fold-out weighted foot): OK?
2. Halo on the **front** around the screen (this draft), or facing **backward** to wash the wall behind? A rear glow is softer but needs a wall close behind.
3. Colour and finish: one colour, or a contrasting foot and bezel?
4. Can you measure E1–E6 with calipers? If not, I work from Waveshare's drawing and you test-fit a quick print.
