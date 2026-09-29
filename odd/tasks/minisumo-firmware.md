# Minisumo Robot Firmware

## Objective
Implement competitive autonomous Minisumo firmware on Arduino Nano (ATmega328P) using a 100% non-blocking Finite State Machine (FSM) tested test-first via the `CombatEngine` seam.

## Context & Hardware
- **Dimensions & Category**: 10x10 cm, 500 g max, Autonomous Amateur, 77 cm Dohyo.
- **Actuators**: 2x yellow TT gearmotors with custom 3D printed narrow wheels, TB6612FNG driver.
- **Sensors**: 
  - Front Ultrasonic HC-SR04 (4.5 ms timeout).
  - 2x Downward TCRT5000 line sensors (A2 Left, A3 Right) via analog readings with software threshold.
- **Start Mechanism**: Tactile pushbutton on D10 with internal pullup (`INPUT_PULLUP`), D13 LED indicator during 5s Start Delay.
- **Pinout**:
  - `PIN_STBY`: 8
  - `PIN_AIN1`: 7
  - `PIN_AIN2`: 9
  - `PIN_PWMA`: 5 (Timer 0 PWM)
  - `PIN_BIN1`: 4
  - `PIN_BIN2`: 2
  - `PIN_PWMB`: 6 (Timer 0 PWM)
  - `PIN_TRIG`: 11
  - `PIN_ECHO`: 12
  - `PIN_TCRT_LEFT`: A2
  - `PIN_TCRT_RIGHT`: A3
  - `PIN_BUTTON`: 10
  - `PIN_LED`: 13

## Tasks
- [x] **task-01**: Define pure domain types and interfaces (`include/CombatTypes.h`)
- [x] **task-02**: TDD Slice 1 - Implement `CombatEngine` state transitions for `WAITING_START` and 5-second `START_DELAY` countdown
- [ ] **task-03**: TDD Slice 2 - Implement `SEARCH` (spin-in-place) and `ATTACK` (direct forward drive) routines
- [ ] **task-04**: TDD Slice 3 - Implement `EVADE` routine (asymmetric 300 ms recovery with ultrasonic suppression)
- [ ] **task-05**: Implement hardware abstraction drivers and pin mapping matching physical wiring
- [ ] **task-06**: Integrate `src/main.cpp` and verify full PlatformIO build for ATmega328P
