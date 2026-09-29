#include <Arduino.h>
#include "HardwareConfig.h"
#include "CombatTypes.h"
#include "CombatEngine.h"
#include "MotorDriverTB6612.h"
#include "UltrasonicSensor.h"
#include "LineSensors.h"

// Hardware driver instances
static MotorDriverTB6612 motors;
static UltrasonicSensor ultrasonic;
static LineSensors line_sensors;
static CombatEngine engine;

// Periodic telemetry & sensor timers
static unsigned long last_telemetry_ms = 0;
static unsigned long last_ultrasonic_ping_ms = 0;
static UltrasonicReading cached_ultrasonic = {0, false};

// Start trigger debounce state
static unsigned long button_press_start_ms = 0;
static bool button_confirmed_pressed = false;

void setup() {
    Serial.begin(115200);
    Serial.println(F("=== MINISUMO FIRMWARE INITIALIZING ==="));

    // Operator interface pins
    pinMode(Pinout::START_TRIGGER, INPUT_PULLUP);
    pinMode(Pinout::LED, OUTPUT);
    digitalWrite(Pinout::LED, LOW);

    // Initialize hardware subsystems
    motors.init();
    ultrasonic.init();
    line_sensors.init();

    Serial.println(F("Hardware initialized. Waiting for Start Trigger (D10)..."));
}

void loop() {
    unsigned long now = millis();
    CombatSensors sensors;
    sensors.current_time_ms = now;

    // 1. Debounced Start Trigger (active LOW via INPUT_PULLUP)
    bool raw_pressed = (digitalRead(Pinout::START_TRIGGER) == LOW);
    if (raw_pressed) {
        if (button_press_start_ms == 0) {
            button_press_start_ms = now;
        } else if (now - button_press_start_ms >= 30) {
            button_confirmed_pressed = true;
        }
    } else {
        button_press_start_ms = 0;
        button_confirmed_pressed = false;
    }
    sensors.start_trigger_active = button_confirmed_pressed;

    // 2. High-frequency line sensor sampling (unblocked, runs every iteration > 4 kHz)
    LineSensorsReading line = line_sensors.read();
    sensors.line_left_raw = line.left_raw;
    sensors.line_right_raw = line.right_raw;

    // 3. Rate-limited ultrasonic sensor reading (50 ms pacing, avoids transducer ringing):
    // Active during COUNTDOWN, SEARCH, and ATTACK so direct post-countdown attack is possible.
    // Strictly suppressed during EVADE (ADR 0001) to protect border escape maneuvers.
    CombatState current_state = engine.getState();
    if (current_state == STATE_START_DELAY || current_state == STATE_SEARCH || current_state == STATE_ATTACK) {
        if (now - last_ultrasonic_ping_ms >= 50) {
            last_ultrasonic_ping_ms = now;
            cached_ultrasonic = ultrasonic.sample();
        }
        sensors.distance_cm = cached_ultrasonic.distance_cm;
        sensors.target_detected = cached_ultrasonic.target_detected;
    } else {
        sensors.distance_cm = 0;
        sensors.target_detected = false;
        cached_ultrasonic.distance_cm = 0;
        cached_ultrasonic.target_detected = false;
    }

    // 4. Update core combat state machine
    engine.update(sensors);

    // 5. Dispatch commands to physical actuators
    motors.setMotors(engine.getMotorCommand());
    digitalWrite(Pinout::LED, engine.isLedActive() ? HIGH : LOW);

    // 6. Optional telemetry output (every 250ms, non-blocking)
    if (now - last_telemetry_ms >= 250) {
        last_telemetry_ms = now;
        Serial.print(F("State: "));
        switch (engine.getState()) {
            case STATE_WAITING_FOR_START: Serial.print(F("WAIT_START")); break;
            case STATE_START_DELAY:       Serial.print(F("COUNTDOWN_5S")); break;
            case STATE_SEARCH:            Serial.print(F("SEARCH")); break;
            case STATE_ATTACK:            Serial.print(F("ATTACK")); break;
            case STATE_EVADE:             Serial.print(F("EVADE")); break;
        }
        Serial.print(F(" | Line L: ")); Serial.print(sensors.line_left_raw);
        Serial.print(F(" R: "));        Serial.print(sensors.line_right_raw);
        Serial.print(F(" | Dist: "));   Serial.print(sensors.distance_cm);
        Serial.print(F(" | Target: ")); Serial.println(sensors.target_detected ? F("YES") : F("NO"));
    }
}
