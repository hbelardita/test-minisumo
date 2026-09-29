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

// Periodic telemetry timer
static unsigned long last_telemetry_ms = 0;

void setup() {
    Serial.begin(115200);
    Serial.println(F("=== MINISUMO FIRMWARE INITIALIZING ==="));

    // Operator interface pins
    pinMode(Pinout::BUTTON, INPUT_PULLUP);
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

    // 1. Operator start button (active LOW via INPUT_PULLUP)
    sensors.start_button_pressed = (digitalRead(Pinout::BUTTON) == LOW);

    // 2. High-frequency line sensor sampling (sampled every iteration > 1 kHz)
    LineSensorsReading line = line_sensors.read();
    sensors.line_left_raw = line.left_raw;
    sensors.line_right_raw = line.right_raw;

    // 3. Ultrasonic sensor reading:
    // Suppressed during EVADE to prevent blocking escape maneuver and ring-outs (ADR 0001)
    if (engine.getState() == STATE_SEARCH || engine.getState() == STATE_ATTACK) {
        UltrasonicReading us = ultrasonic.sample();
        sensors.distance_cm = us.distance_cm;
        sensors.target_detected = us.target_detected;
    } else {
        sensors.distance_cm = 0;
        sensors.target_detected = false;
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
