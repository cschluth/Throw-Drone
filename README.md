# Throw-Drone

A small, hand-throwable 65 mm quadcopter that will detect a throw, stabilize itself mid-air, and hover.

https://github.com/user-attachments/assets/fbd82480-2b36-4a60-ba84-f2fc82b2668f

*First flight on the PLA prototype frame.*

<img src="https://github.com/user-attachments/assets/6309ddb2-4f77-4d76-8838-304be4f29027" alt="The assembled drone" width="450">

*Phase 1 build: the assembled quadcopter on the PLA prototype frame.*

## Status

**Phase 1: Base quadcopter (almost complete)**
- All four motors soldered to the ESC and configured in Betaflight
- Receiver wired to the flight controller (RX and TX)
- Successful first flight on a PLA prototype frame
- Tuning PIDs in Betaflight to make the flight more controllable (in progress)

**Phase 2: Throw detection (planned)**
- Finalize and reprint the frame in PETG-CF
- Wire the ESP32-S3 to the flight controller
- Test and tune the throw-detection logic on real hardware

Started June 2026. This is a work in progress, and I update this page as I go.

## Why I'm building it

I'm a mechanical engineering student, and I wanted a project of my own outside of class. I'm interested in electronics, and I wanted something that combined that with the more traditional mechanical side, like CAD, 3D printing, and building prototypes. A drone was a good fit because it needs both: a frame that has to be designed and printed, and electronics that have to be wired, configured, and programmed to work with it. I added the throw-and-hover idea because I thought it would be cool, and it gives the drone a trick of its own instead of just flying around. I'm learning most of this as I go, so I'm building it in stages, starting with a simple version that flies and adding the harder parts after.

## How it will work

1. An IMU detects that the drone has been thrown.
2. The motors spool up.
3. The flight controller recovers the drone's attitude.
4. The drone stabilizes and hovers.

The logic is a five-state machine: `IDLE`, `HELD`, `RELEASED`, `FREEFALL`, `STABILIZING`.

## Hardware

| Part | Model |
|---|---|
| Flight controller / ESC | SpeedyBee F405 Mini BLS 35A stack |
| Motors | Happymodel RS1102 10000KV (2S) |
| Battery | OVONIC 2S 450 mAh LiPo (XT30) |
| Props | HQProp 65 mm |
| Receiver | ELRS C8 nano |
| Microcontroller | Seeed XIAO ESP32-S3 |
| 3D printer | Elegoo Centauri Carbon |

Weight: ~80 g (estimated)

## Design

- Designed in Fusion 360
- Layered frame: top plate, a structural middle plate with tapered arms, and a battery bay below
- Through-bolts with captured hex nut pockets, since snap fits were impractical at this scale
- PLA prototypes for fit and flight checks, then PETG-CF for the final frame

### CAD: mid plate

<table>
  <tr>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/0e1f19c3-0420-464c-b8f6-c8fdde986aff" alt="Mid plate with visible edges" width="420"><br>
      <sub>Mid plate: visible edges</sub>
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/4ef5ad53-7689-499b-9c58-a0482d91dc39" alt="Mid plate with hidden edges" width="420"><br>
      <sub>Mid plate: hidden edges</sub>
    </td>
  </tr>
</table>

### Prototype frames

<table>
  <tr>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/0ac47e04-ee48-4a17-bacc-78ab0f607c18" alt="Prototype frame 1" width="320"><br>
      <sub>Frame prototype 1</sub>
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/4e1a4733-6815-4194-8796-28f4f84e8d7a" alt="Prototype frame 2" width="320"><br>
      <sub>Frame prototype 2</sub>
    </td>
  </tr>
  <tr>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/fd4d78fc-54f9-4119-b5fc-43111d148001" alt="Prototype frame 3" width="320"><br>
      <sub>Frame prototype 3</sub>
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/32b2d740-d292-47c1-b2cd-7333af1ad14d" alt="Prototype frame 4" width="320"><br>
      <sub>Frame prototype 4</sub>
    </td>
  </tr>
</table>

## Software

The ESP32-S3 firmware is in the [`code`](code) folder, written in Arduino C++.

**Note:** The initial version of this code was written with Claude AI assistance. I designed the state machine and am working through the code to understand how it works. The IMU input is currently simulated for bench testing, and nothing has been tested on the drone yet.

## What I've learned so far

- **Betaflight settings can silently fail to save in the GUI.** The CLI is more reliable for critical config like serial ports.
- **The FC's 5V pad is battery-powered, not USB-powered.** Only 3.3V is live over USB, which is why my receiver showed no activity during USB-only testing.
- **UART wiring is always crossed.** Receiver TX goes to FC RX, and receiver RX goes to FC TX.
- **PLA is fine for setup and low-throttle checks, but not for throw tests.** It's brittle and weak near the motors, so the final frame will be PETG-CF.

## Roadmap

- [x] Select parts and design the frame
- [x] Print PLA prototype and test-fit electronics
- [x] Solder and configure all four motors
- [x] First flight
- [ ] Tune PIDs in Betaflight (in progress)
- [ ] Finish frame design
- [ ] Reprint in PETG-CF
- [ ] Add prop guards before throw tests
- [ ] Wire ESP32-S3 to the flight controller
- [ ] Test throw detection on hardware
- [ ] Document the minimum safe throw height

## Contact

Charlie Schluth, Mechanical Engineering, Penn State
