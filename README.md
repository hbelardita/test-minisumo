# Autonomous Minisumo Robot Firmware

Autonomous Minisumo robot firmware for the Amateur category (10x10 cm, 500 g max, 77 cm Dohyo). Built on Arduino Nano (ATmega328P) using a 100% non-blocking Finite State Machine (FSM) tested test-first via pure domain logic.

## Hardware & Pinout

| Subsystem | Component | MCU Pin | Notes |
|---|---|---|---|
| **Motors (Right)** | TB6612FNG (Channel A) | D5 (PWMA), D7 (AIN1), D9 (AIN2) | Timer 0 PWM |
| **Motors (Left)** | TB6612FNG (Channel B) | D6 (PWMB), D4 (BIN1), D2 (BIN2) | Timer 0 PWM |
| **Motor Driver STBY** | TB6612FNG | D8 | Active HIGH, LOW = standby |
| **Ultrasonic Sensor** | HC-SR04 | D11 (TRIG), D12 (ECHO) | 4.5 ms timeout (~77 cm max travel) |
| **Line Sensors** | 2x TCRT5000 | A2 (Left), A3 (Right) | Analog input, threshold: 500 |
| **Start Trigger** | Tactile button | D10 | Active LOW (`INPUT_PULLUP`) |
| **Status Indicator** | Onboard LED | D13 | Blinks during 5s delay |

## Firmware Environments & Commands

The project uses [PlatformIO](https://platformio.org/) with isolated build environments in [platformio.ini](platformio.ini).

### 1. Competition Mode (`nanoatmega328` - Default)

Full autonomous combat firmware running the non-blocking state machine (`WAITING_FOR_START` -> `START_DELAY` (5s) -> `SEARCH` / `ATTACK` / `EVADE`). Telemetry over Serial is disabled in this mode to eliminate UART interrupt overhead and maximize MCU performance during competition.

```bash
# Build default competition firmware
pio run

# Build and upload to Arduino Nano
pio run -t upload
```

### 2. Bench Sensor Calibration Mode (`sensor_monitor`)

Dedicated bench testing tool. Actuators remain safely shut down (`STBY` held LOW). Live sensor readings (analog line reflectance, ultrasonic distance, button state) stream continuously over serial for threshold calibration.

```bash
# Build, upload, and monitor sensor calibration mode in one command
pio run -e sensor_monitor -t upload -t monitor
```

**Telemetry output format:**
```text
[MONITOR] Line L: 620 (BLACK) | Line R: 110 (WHITE) | Dist: 18 cm (TARGET) | Btn: UP
```

## Running Unit Tests

The core combat state machine (`CombatEngine`) is decoupled from hardware drivers and tested natively without microcontroller hardware:

```bash
# Compile and run native combat engine test suite
g++ -std=c++11 -Iinclude test/test_combat_engine.cpp -o test/test_combat_engine && ./test/test_combat_engine
```

## Architecture & Decisions

- Pure domain logic isolated in [`include/CombatEngine.h`](include/CombatEngine.h) and [`include/CombatTypes.h`](include/CombatTypes.h).
- Non-blocking loop driven by `millis()` with microsecond-level ultrasonic suppression. See [ADR-0001: Non-blocking FSM and Ultrasonic Suppression](docs/adr/0001-non-blocking-fsm-and-ultrasonic-suppression.md).
