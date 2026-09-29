#include <Arduino.h>
#include "HardwareConfig.h"
#include "LineSensors.h"
#include "UltrasonicSensor.h"
#include "MotorDriverTB6612.h"
#include "CombatTypes.h"

// Hardware driver instances
static MotorDriverTB6612 motors;
static UltrasonicSensor ultrasonic;
static LineSensors line_sensors;

// Combat configuration reference for thresholds
static const CombatConfig combat_config;

// Periodic telemetry timer
static unsigned long last_telemetry_ms = 0;
constexpr unsigned long TELEMETRY_INTERVAL_MS = 100;

void setup() {
    Serial.begin(115200);
    Serial.println(F("=== MINISUMO SENSOR MONITOR / CALIBRATION MODE ==="));
    Serial.println(F("Motors disabled in standby. Telemetry streaming at 100ms."));

    // Operator interface pins
    pinMode(Pinout::START_TRIGGER, INPUT_PULLUP);
    pinMode(Pinout::LED, OUTPUT);
    digitalWrite(Pinout::LED, LOW);

    // Initialize motor driver and ensure absolute hardware shutdown
    motors.init();
    motors.stop();

    // Initialize sensor peripherals
    ultrasonic.init();
    line_sensors.init();

    Serial.println(F("Sensors ready. Format: [MONITOR] Line L | Line R | Dist | Btn"));
}

void loop() {
    unsigned long now = millis();

    if (now - last_telemetry_ms >= TELEMETRY_INTERVAL_MS) {
        last_telemetry_ms = now;

        // Sample line sensors
        LineSensorsReading line = line_sensors.read();
        const bool left_is_white = (line.left_raw < combat_config.line_white_threshold);
        const bool right_is_white = (line.right_raw < combat_config.line_white_threshold);

        // Sample ultrasonic sensor (max detection range from config)
        UltrasonicReading us = ultrasonic.sample(combat_config.ultrasonic_max_distance_cm);

        // Sample push button (active LOW)
        const bool button_pressed = (digitalRead(Pinout::START_TRIGGER) == LOW);

        // Stream formatted dashboard telemetry
        Serial.print(F("[MONITOR] Line L: "));
        Serial.print(line.left_raw);
        Serial.print(left_is_white ? F(" (WHITE)") : F(" (BLACK)"));

        Serial.print(F(" | Line R: "));
        Serial.print(line.right_raw);
        Serial.print(right_is_white ? F(" (WHITE)") : F(" (BLACK)"));

        Serial.print(F(" | Dist: "));
        if (us.target_detected) {
            Serial.print(us.distance_cm);
            Serial.print(F(" cm (TARGET)"));
        } else {
            Serial.print(F("--- cm (CLEAR)"));
        }

        Serial.print(F(" | Btn: "));
        Serial.println(button_pressed ? F("PRESSED") : F("UP"));
    }
}
