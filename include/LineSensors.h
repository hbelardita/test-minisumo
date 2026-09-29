#ifndef LINE_SENSORS_H
#define LINE_SENSORS_H

#include <Arduino.h>
#include "HardwareConfig.h"

struct LineSensorsReading {
    uint16_t left_raw;
    uint16_t right_raw;
};

class LineSensors {
public:
    void init() {
        pinMode(Pinout::TCRT_LEFT, INPUT);
        pinMode(Pinout::TCRT_RIGHT, INPUT);
    }

    LineSensorsReading read() {
        LineSensorsReading reading;
        reading.left_raw = analogRead(Pinout::TCRT_LEFT);
        reading.right_raw = analogRead(Pinout::TCRT_RIGHT);
        return reading;
    }
};

#endif // LINE_SENSORS_H
