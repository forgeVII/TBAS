# YouTube Video Details — Upload as Unlisted

> Replace VIDEO_LINK placeholders with actual YouTube URLs after uploading.

---

## Video 1: Acoustic Detection Demo

**Title:** TBAS — Acoustic Direction Finding with Arduino (Live Demo)

**Description:**
```
Live demo of TBAS (Tactical Battlefield Awareness Suite) — an Arduino Nano based acoustic direction finding system.

Two electret microphones spaced 28cm apart detect sound. TDOA (Time Difference of Arrival) physics calculates the angle. A servo rotates toward the source. An IR system identifies friend or foe. A browser-based radar displays everything in real time.

No GPS. No cameras. No complex processors. Just sound, math, and IR light.

Hardware:
- Arduino Nano (ATmega328P)
- 2× electret microphones with LM393 comparator
- Servo motor
- TSOP1838 IR receiver + IR LED
- RGB LED status indicator
- Web Serial API interface (Chrome/Edge)

Full documentation: https://github.com/forgeVII-org/TBAS

#Arduino #AcousticDetection #TDOA #IFF #EmbeddedSystems #DIY
```

---

## Video 2: Web Interface Walkthrough

**Title:** TBAS Web Interface — Live Radar, IFF Display, Mode Control (Web Serial API)

**Description:**
```
Walking through the TBAS web interface — a browser-based radar display that connects to Arduino over USB using Web Serial API.

Features shown:
- Live animated radar with servo sweep
- IFF (Identification Friend or Foe) status display
- Real-time bearing angle with needle indicator
- Mode switching (Manual, Surveillance, Servo Test, Acoustic)
- TX mode toggle (Friend/Enemy)
- Timestamped event log
- Auto-reconnect on disconnect

No drivers. No server. No installation. Just open in Chrome and click Connect.

Full documentation: https://github.com/forgeVII-org/TBAS

#WebSerial #Arduino #Radar #IFF #EmbeddedSystems
```

---

## Video 3: IFF Rolling Code System

**Title:** TBAS — Dynamic IFF Rolling Code System (NEC IR Protocol)

**Description:**
```
Explaining the IFF (Identification Friend or Foe) system in TBAS — uses NEC protocol IR communication with a dynamic rolling code instead of a fixed static code.

code = ( KEY × counter + 5 ) mod 256

Both units share the same KEY and maintain a synchronized counter. The counter increments on every successful Friend exchange, so the code changes every cycle. An intercepted code cannot be replayed because the expected code has already advanced.

Demonstrated:
- Friend TX mode (correct code) → GREEN
- Enemy TX mode (wrong code) → RED
- No response → BLUE
- Rolling code counter increment

Full documentation: https://github.com/forgeVII-org/TBAS

#IFF #RollingCode #NEC #IR #Arduino #Security
```

---

## Video 4: Build Process

**Title:** Building TBAS — Arduino Acoustic Direction Finder from Scratch

**Description:**
```
Full build process of TBAS (Tactical Battlefield Awareness Suite) — from bare Arduino Nano to working acoustic direction finder.

What you'll see:
- Arduino Nano and component overview
- Microphone circuit with LM393 comparator
- 28cm mic spacing calibration
- Servo mounting with TSOP1838 IR receiver
- RGB LED and button wiring
- Firmware upload and first boot
- Web interface connection and testing
- Live acoustic detection test

Total cost: ~$10-15 (all components from AliExpress/local market)

Full documentation: https://github.com/forgeVII-org/TBAS

#Arduino #BuildProcess #AcousticDetection #DIY #EmbeddedSystems
```

---

## Upload Settings (Same for All)

- **Visibility:** Unlisted
- **Category:** Science & Technology
- **Comments:** On
- **License:** Creative Commons — Attribution
- **Playlist:** Create "TBAS — Tactical Battlefield Awareness Suite" playlist and add all 4
