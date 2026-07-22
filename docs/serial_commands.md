# Serial Commands — TBAS

## Connection

- **Baud Rate:** 9600
- **Line Ending:** Newline (`\n`)
- **Protocol:** ASCII text

---

## Commands

### System

| Command | Response | Description |
|---|---|---|
| `PING` | `PONG` + status | Heartbeat check |
| `STATUS` | `STATUS: MODE=... TX=... ANGLE=... IFF=...` | Full status dump |

### Mode Control

| Command | Response | Description |
|---|---|---|
| `MODE:MANUAL` | `MODE: MANUAL` | Servo controlled by buttons |
| `MODE:SURV` | `MODE: SURVEILLANCE` | Auto-sweep with IFF |
| `MODE:ACO` | `MODE: ACOUSTIC` + `ACO READY` | Acoustic detection mode |
| `MODE:TEST` | `MODE: SERVO_TEST` + `TBAS SERVO TEST READY` | Calibration/diagnostic mode |

### TX Mode

| Command | Response | Description |
|---|---|---|
| `TX:FRIEND` | `TX: FRIEND` | Send correct rolling code |
| `TX:ENEMY` | `TX: ENEMY` | Send incorrect rolling code |

### Servo Control

| Command | Response | Description |
|---|---|---|
| `ANGLE:XXX` | `Angle: XXX` | Set servo angle (25-155, Manual mode only) |

---

## Responses

### Status Format

```
STATUS: MODE=MANUAL TX=FRIEND ANGLE=90 IFF=0
```

Fields:
- `MODE` — Current mode (MANUAL, SURVEILLANCE, SERVO_TEST, ACOUSTIC)
- `TX` — Transmit mode (FRIEND, ENEMY)
- `ANGLE` — Current servo angle (25-155)
- `IFF` — IFF counter (incremented on successful Friend exchange)

### IFF Results

```
IFF: FRIEND DIST:42cm
IFF: ENEMY DIST:15cm
IFF: NO RESPONSE
```

### Acoustic Detection

```
ACO ANGLE: 120
```

### Surveillance

```
SURV TARGET: 85
```

### Servo Test

```
TDOA=342us | Angle=115deg
```

### Noise Rejection

```
DISCARDED: 1205
```

### Lockout

```
LOCKOUT
ACO READY
```

---

## Heartbeat

The Arduino sends a status heartbeat every 5 seconds:

```
STATUS: MODE=MANUAL TX=FRIEND ANGLE=90 IFF=0
```

The web interface also sends `PING` every 4 seconds and expects `PONG` within 12 seconds, otherwise it triggers a disconnect.

---

## Web Interface Commands

The web interface (`tbas_interface.html`) sends these commands automatically:

1. On connect: `TX:FRIEND`, `MODE:MANUAL`, `ANGLE:90`
2. Every 4 seconds: `PING`
3. On button click: `MODE:MANUAL`, `MODE:SURV`, `MODE:TEST`, `MODE:ACO`, `TX:FRIEND`, `TX:ENEMY`
4. On slider change: `ANGLE:XXX`
