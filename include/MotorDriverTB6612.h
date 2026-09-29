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
        setChannelA(cmd.speed_left);
        setChannelB(cmd.speed_right);
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
    void setChannelA(int16_t speed) {
        if (speed > 0) {
            digitalWrite(Pinout::AIN1, HIGH);
            digitalWrite(Pinout::AIN2, LOW);
            analogWrite(Pinout::PWMA, constrain(speed, 0, 255));
        } else if (speed < 0) {
            digitalWrite(Pinout::AIN1, LOW);
            digitalWrite(Pinout::AIN2, HIGH);
            analogWrite(Pinout::PWMA, constrain(-speed, 0, 255));
        } else {
            // Active brake
            digitalWrite(Pinout::AIN1, HIGH);
            digitalWrite(Pinout::AIN2, HIGH);
            analogWrite(Pinout::PWMA, 0);
        }
    }

    void setChannelB(int16_t speed) {
        if (speed > 0) {
            digitalWrite(Pinout::BIN1, HIGH);
            digitalWrite(Pinout::BIN2, LOW);
            analogWrite(Pinout::PWMB, constrain(speed, 0, 255));
        } else if (speed < 0) {
            digitalWrite(Pinout::BIN1, LOW);
            digitalWrite(Pinout::BIN2, HIGH);
            analogWrite(Pinout::PWMB, constrain(-speed, 0, 255));
        } else {
            // Active brake
            digitalWrite(Pinout::BIN1, HIGH);
            digitalWrite(Pinout::BIN2, HIGH);
            analogWrite(Pinout::PWMB, 0);
        }
    }
};

#endif // MOTOR_DRIVER_TB6612_H
