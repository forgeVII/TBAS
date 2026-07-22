# Wiring Reference — TBAS

## Complete Pin Map

| Pin | Direction | Function | Notes |
|---|---|---|---|
| D2 | INPUT | Mic 1 interrupt | RISING edge, hardware interrupt |
| D3 | INPUT | Mic 2 interrupt | RISING edge, hardware interrupt |
| D4 | INPUT | TSOP1838 IR receiver | 38 kHz, no resistor on signal |
| D5 | OUTPUT | IR LED transmit | 100Ω series resistor |
| D8 | INPUT | Button Left | INPUT_PULLUP, active-low |
| D9 | INPUT | Button Right | INPUT_PULLUP, active-low |
| D10 | OUTPUT | TM1637 CLK | Optional — 7-segment display |
| D11 | OUTPUT | TM1637 DIO | Optional — 7-segment display |
| D12 | OUTPUT | Servo PWM | Standard servo signal |
| A3 | OUTPUT | RGB Red | 220Ω series resistor |
| A4 | OUTPUT | RGB Green | 220Ω series resistor |
| A5 | OUTPUT | RGB Blue | 220Ω series resistor |
| A0 | INPUT | Random seed | analogRead for randomSeed() |
| 5V | POWER | Mic modules, servo, IR | From USB or power bank |
| GND | POWER | Common ground | All modules share ground |

---

## Microphone Circuit

```
         5V
          |
         [10kΩ]  ← bias resistor
          |
    Mic (+)──────→ LM393 Pin 3 (IN+)
          |
         GND

    Pot Wiper ────→ LM393 Pin 2 (IN−)
    Pot End 1 ────→ Voltage Divider (10kΩ–10kΩ from 5V to GND)
    Pot End 2 ────→ GND

    LM393 Pin 1 (OUT) ────→ Arduino D2/D3
                    |
                   [10kΩ] ← pullup to 5V
```

**IMPORTANT:** Mic (+) connects to IN+ (Pin 3), pot wiper to IN− (Pin 2). This is opposite to many online tutorials. Interrupt must be configured as RISING, not FALLING.

---

## TSOP1838 IR Receiver

```
With flat face toward you, pins left to right:

  OUT ────→ Arduino D4
  GND ────→ GND
  VCC ────→ 5V
            |
           [100nF] ← decoupling cap
            |
           GND
```

No resistor needed on the signal pin. The TSOP1838 has internal pull-up.

---

## IR LED Circuit

```
Arduino D5 ────[100Ω]──── IR LED (+) ──── GND
```

940nm IR LED. 100Ω series resistor limits current to ~30mA from 5V.

---

## Servo Wiring

```
Servo Wire Color:
  Brown/Black ────→ GND
  Red ────────────→ 5V
  Orange/Yellow ──→ Arduino D12
```

**Power note:** Servo draws significant current on movement. Place a 100µF electrolytic capacitor across servo power pins (Red to Brown/Black). Power from laptop USB is sufficient for lab use. For field deployment, use a power bank.

---

## Button Wiring

```
Arduino D8/D9 ──── Button ──── GND
     (internal pullup enabled)
```

Button pressed = LOW. Button released = HIGH.

---

## RGB LED Wiring

```
Arduino A3 ────[220Ω]──── RGB Red   ────┐
Arduino A4 ────[220Ω]──── RGB Green ────┤ Common
Arduino A5 ────[220Ω]──── RGB Blue  ────┘ Cathode → GND
```

220Ω series resistors on each channel. Common cathode configuration.

---

## Microphone Spacing

The two electret microphones must be spaced exactly **28 cm apart**. This distance determines the maximum detectable TDOA (816 µs) and the angular resolution of the system.

```
   ←────── 28 cm ──────→
   [Mic 1]             [Mic 2]
     D2                   D3
```

Mount both microphones at the same height, facing the same direction, with clear line of sight to sound sources.

---

## Power Distribution

| Source | Voltage | Used By |
|---|---|---|
| USB (laptop/power bank) | 5V | Arduino, mic modules, IR |
| Arduino 5V pin | 5V | Servo (with 100µF cap) |
| Arduino 3.3V pin | 3.3V | Not used |

**Total current draw:** ~100mA idle, ~300mA during servo movement.

---

## Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| No mic interrupts | Wrong interrupt mode | Change from FALLING to RISING |
| Erratic servo movement | Insufficient power | Add 100µF cap across servo |
| IFF always NO RESPONSE | Wrong TSOP orientation | Check pinout: OUT-GND-VCC |
| Servo jitter at 90° | No sound detected | Normal — center position is default |
| Web interface won't connect | Wrong browser | Use Chrome or Edge only |
