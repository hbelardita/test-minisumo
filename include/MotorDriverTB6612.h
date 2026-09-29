#ifndef MOTOR_DRIVER_TB6612_H
#define MOTOR_DRIVER_TB6612_H

#include <Arduino.h>
#include "HardwareConfig.h"
#include "CombatTypes.h"

class MotorDriverTB6612 {
public:
    void init() {
        pinMode(Pinout::STBY, OUTPUT);
        pinMode(Pinout::AIN1, OUTPUT);
        pinMode(Pinout::AIN2, OUTPUT);
        pinMode(Pinout::PWMA, OUTPUT);
        pinMode(Pinout::BIN1, OUTPUT);
        pinMode(Pinout::BIN2, OUTPUT);
        pinMode(Pinout::PWMB, OUTPUT);

        stop();
    }

    void setMotors(const MotorCommand& cmd) {
        if (cmd.standby) {
            digitalWrite(Pinout::STBY, LOW);
            analogWrite(Pinout::PWMA, 0);
            analogWrite(Pinout::PWMB, 0);
            return;
        }

        digitalWrite(Pinout::STBY, HIGH);
        // Canal A: Motor Derecho con polaridad física invertida (-rightSpeed)
        setChannel(Pinout::AIN1, Pinout::AIN2, Pinout::PWMA, -cmd.speed_right);
        // Canal B: Motor Izquierdo con polaridad física invertida (-leftSpeed)
        setChannel(Pinout::BIN1, Pinout::BIN2, Pinout::PWMB, -cmd.speed_left);
    }

    void stop() {
        digitalWrite(Pinout::STBY, LOW);
        analogWrite(Pinout::PWMA, 0);
        analogWrite(Pinout::PWMB, 0);
        digitalWrite(Pinout::AIN1, LOW);
        digitalWrite(Pinout::AIN2, LOW);
        digitalWrite(Pinout::BIN1, LOW);
        digitalWrite(Pinout::BIN2, LOW);
    }

private:
    void setChannel(uint8_t in1, uint8_t in2, uint8_t pwm_pin, int16_t speed) {
        if (speed > 0) {
            digitalWrite(in1, HIGH);
            digitalWrite(in2, LOW);
            analogWrite(pwm_pin, constrain(speed, 0, 255));
        } else if (speed < 0) {
            digitalWrite(in1, LOW);
            digitalWrite(in2, HIGH);
            analogWrite(pwm_pin, constrain(-speed, 0, 255));
        } else {
            // Active short brake
            digitalWrite(in1, HIGH);
            digitalWrite(in2, HIGH);
            analogWrite(pwm_pin, 0);
        }
    }
};

#endif // MOTOR_DRIVER_TB6612_H
