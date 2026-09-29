#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>
#include "HardwareConfig.h"

struct UltrasonicReading {
    uint16_t distance_cm;
    bool target_detected;
};

class UltrasonicSensor {
public:
    void init() {
        pinMode(Pinout::TRIG, OUTPUT);
        pinMode(Pinout::ECHO, INPUT);
        digitalWrite(Pinout::TRIG, LOW);
    }

    // Strictly capped timeout (4500 us = ~77 cm max travel in Dohyo)
    // Avoids freezing CPU and blinding line sensors
    UltrasonicReading sample(uint16_t max_range_cm = 70) {
        UltrasonicReading reading;
        reading.distance_cm = 0;
        reading.target_detected = false;

        // Trigger 10us pulse
        digitalWrite(Pinout::TRIG, LOW);
        delayMicroseconds(2);
        digitalWrite(Pinout::TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(Pinout::TRIG, LOW);

        // 4500 us timeout (~77 cm round trip)
        unsigned long duration = pulseIn(Pinout::ECHO, HIGH, 4500);

        if (duration > 0) {
            uint16_t cm = duration / 58;
            if (cm > 0 && cm <= max_range_cm) {
                reading.distance_cm = cm;
                reading.target_detected = true;
            }
        }

        return reading;
    }
};

#endif // ULTRASONIC_SENSOR_H
