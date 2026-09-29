#ifndef COMBAT_ENGINE_H
#define COMBAT_ENGINE_H

#include "CombatTypes.h"

class CombatEngine {
public:
    CombatEngine(const CombatConfig& config = CombatConfig()) :
        m_config(config),
        m_state(STATE_WAITING_FOR_START),
        m_countdown_start_ms(0),
        m_evade_start_ms(0),
        m_evade_direction(EVADE_NONE),
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

    EvadeDirection getEvadeDirection() const {
        return m_evade_direction;
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
                if (sensors.start_trigger_active) {
                    m_state = STATE_START_DELAY;
                    m_countdown_start_ms = sensors.current_time_ms;
                    m_led_active = ((sensors.current_time_ms / 250) % 2) == 0;
                }
                break;

            case STATE_START_DELAY:
                m_motor_cmd.speed_left = 0;
                m_motor_cmd.speed_right = 0;
                m_motor_cmd.standby = true;
                // Blink LED every 250ms during countdown
                m_led_active = ((sensors.current_time_ms / 250) % 2) == 0;
                if (sensors.current_time_ms - m_countdown_start_ms >= m_config.start_delay_ms) {
                    if (sensors.target_detected) {
                        enterAttack();
                    } else {
                        enterSearch();
                    }
                }
                break;

            case STATE_SEARCH:
                if (checkLineBorder(sensors)) {
                    // Evade entered
                } else if (sensors.target_detected) {
                    enterAttack();
                } else {
                    applySearchMotors();
                }
                break;

            case STATE_ATTACK:
                if (checkLineBorder(sensors)) {
                    // Evade entered
                } else if (!sensors.target_detected) {
                    enterSearch();
                } else {
                    applyAttackMotors();
                }
                break;

            case STATE_EVADE:
                // Ultrasonic is completely suppressed during evade!
                if (sensors.current_time_ms - m_evade_start_ms >= m_config.evade_duration_ms) {
                    enterSearch();
                } else {
                    applyEvadeMotors(sensors.current_time_ms);
                }
                break;
        }
    }

private:
    bool checkLineBorder(const CombatSensors& sensors) {
        bool left_white = (sensors.line_left_raw < m_config.line_white_threshold);
        bool right_white = (sensors.line_right_raw < m_config.line_white_threshold);

        if (left_white && right_white) {
            enterEvade(EVADE_FULL_TURN, sensors.current_time_ms);
            return true;
        } else if (left_white) {
            enterEvade(EVADE_TURN_RIGHT, sensors.current_time_ms);
            return true;
        } else if (right_white) {
            enterEvade(EVADE_TURN_LEFT, sensors.current_time_ms);
            return true;
        }
        return false;
    }

    void enterSearch() {
        m_state = STATE_SEARCH;
        m_evade_direction = EVADE_NONE;
        m_led_active = true;
        applySearchMotors();
    }

    void applySearchMotors() {
        m_motor_cmd.standby = false;
        m_motor_cmd.speed_left = -m_config.search_speed;
        m_motor_cmd.speed_right = m_config.search_speed;
    }

    void enterAttack() {
        m_state = STATE_ATTACK;
        m_evade_direction = EVADE_NONE;
        m_led_active = true;
        applyAttackMotors();
    }

    void applyAttackMotors() {
        m_motor_cmd.standby = false;
        m_motor_cmd.speed_left = m_config.attack_speed;
        m_motor_cmd.speed_right = m_config.attack_speed;
    }

    void enterEvade(EvadeDirection dir, uint32_t current_time_ms) {
        m_state = STATE_EVADE;
        m_evade_direction = dir;
        m_evade_start_ms = current_time_ms;
        applyEvadeMotors(current_time_ms);
    }

    void applyEvadeMotors(uint32_t current_time_ms) {
        m_motor_cmd.standby = false;
        uint32_t elapsed = current_time_ms - m_evade_start_ms;
        uint32_t half_duration = m_config.evade_duration_ms / 2;

        if (elapsed < half_duration) {
            // Phase 1: Reverse away from edge
            m_motor_cmd.speed_left = -m_config.reverse_speed;
            m_motor_cmd.speed_right = -m_config.reverse_speed;
        } else {
            // Phase 2: Asymmetric turn away from edge
            switch (m_evade_direction) {
                case EVADE_TURN_RIGHT:
                    m_motor_cmd.speed_left = m_config.search_speed;
                    m_motor_cmd.speed_right = -m_config.search_speed;
                    break;
                case EVADE_TURN_LEFT:
                    m_motor_cmd.speed_left = -m_config.search_speed;
                    m_motor_cmd.speed_right = m_config.search_speed;
                    break;
                case EVADE_FULL_TURN:
                default:
                    m_motor_cmd.speed_left = m_config.search_speed;
                    m_motor_cmd.speed_right = -m_config.search_speed;
                    break;
            }
        }
    }

    CombatConfig m_config;
    CombatState m_state;
    uint32_t m_countdown_start_ms;
    uint32_t m_evade_start_ms;
    EvadeDirection m_evade_direction;
    MotorCommand m_motor_cmd;
    bool m_led_active;
};

#endif // COMBAT_ENGINE_H
