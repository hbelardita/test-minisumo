#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <stdint.h>

namespace Pinout {
    // TB6612FNG Dual Motor Driver
    constexpr uint8_t STBY = 8;
    constexpr uint8_t AIN1 = 7;
    constexpr uint8_t AIN2 = 9;
    constexpr uint8_t PWMA = 5; // Timer 0 PWM
    constexpr uint8_t BIN1 = 4;
    constexpr uint8_t BIN2 = 2;
    constexpr uint8_t PWMB = 6; // Timer 0 PWM

    // Ultrasonic Sensor HC-SR04
    constexpr uint8_t TRIG = 11;
    constexpr uint8_t ECHO = 12;

    // TCRT5000 Analog Line Sensors
    constexpr uint8_t TCRT_LEFT = A2;
    constexpr uint8_t TCRT_RIGHT = A3;

    // Operator Interface
    constexpr uint8_t START_TRIGGER = 10;
    constexpr uint8_t LED = 13;
}

#endif // HARDWARE_CONFIG_H
