#ifndef COMBAT_ENGINE_H
#define COMBAT_ENGINE_H

#include "CombatTypes.h"

class CombatEngine {
public:
    CombatEngine(const CombatConfig& config = CombatConfig()) :
        m_config(config),
        m_state(STATE_WAITING_FOR_START),
        m_countdown_start_ms(0),
        m_led_active(false) {
        m_motor_cmd.speed_left = 0;
        m_motor_cmd.speed_right = 0;
        m_motor_cmd.standby = true;
    }

    CombatState getState() const {
        return m_state;
    }

    MotorCommand getMotorCommand() const {
        return m_motor_cmd;
    }

    bool isLedActive() const {
        return m_led_active;
    }

    void update(const CombatSensors& sensors) {
        switch (m_state) {
            case STATE_WAITING_FOR_START:
                m_motor_cmd.speed_left = 0;
                m_motor_cmd.speed_right = 0;
                m_motor_cmd.standby = true;
                m_led_active = false;
                if (sensors.start_button_pressed) {
                    m_state = STATE_START_DELAY;
                    m_countdown_start_ms = sensors.current_time_ms;
                }
                break;

            case STATE_START_DELAY:
                m_motor_cmd.speed_left = 0;
                m_motor_cmd.speed_right = 0;
                m_motor_cmd.standby = true;
                // Blink LED every 250ms during countdown
                m_led_active = ((sensors.current_time_ms / 250) % 2) == 0;
                if (sensors.current_time_ms - m_countdown_start_ms >= m_config.start_delay_ms) {
                    m_state = STATE_SEARCH;
                    m_led_active = true;
                }
                break;

            case STATE_SEARCH:
            case STATE_ATTACK:
            case STATE_EVADE:
                break;
        }
    }

private:
    CombatConfig m_config;
    CombatState m_state;
    uint32_t m_countdown_start_ms;
    MotorCommand m_motor_cmd;
    bool m_led_active;
};

#endif // COMBAT_ENGINE_H
