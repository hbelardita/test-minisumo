#ifndef COMBAT_TYPES_H
#define COMBAT_TYPES_H

#include <stdint.h>

enum CombatState {
    STATE_WAITING_FOR_START,
    STATE_START_DELAY,
    STATE_SEARCH,
    STATE_ATTACK,
    STATE_EVADE
};

enum EvadeDirection {
    EVADE_NONE,
    EVADE_TURN_RIGHT, // Left sensor hit Border Line
    EVADE_TURN_LEFT,  // Right sensor hit Border Line
    EVADE_FULL_TURN   // Both sensors hit Border Line
};

struct MotorCommand {
    int16_t speed_left;   // -255 to 255 (negative = reverse)
    int16_t speed_right;  // -255 to 255 (negative = reverse)
    bool standby;         // true = motors disabled / floating

    bool operator==(const MotorCommand& other) const {
        return speed_left == other.speed_left &&
               speed_right == other.speed_right &&
               standby == other.standby;
    }
};

struct CombatSensors {
    bool start_trigger_active;
    uint16_t line_left_raw;
    uint16_t line_right_raw;
    uint16_t distance_cm;
    bool target_detected;
    uint32_t current_time_ms;

    CombatSensors() :
        start_trigger_active(false),
        line_left_raw(800),  // Default: safely on black Dohyo surface
        line_right_raw(800), // Default: safely on black Dohyo surface
        distance_cm(0),
        target_detected(false),
        current_time_ms(0) {}
};

struct CombatConfig {
    uint32_t start_delay_ms;
    uint32_t evade_duration_ms;
    uint16_t line_white_threshold;
    uint16_t ultrasonic_max_distance_cm;
    int16_t search_speed;
    int16_t attack_speed;
    int16_t reverse_speed;

    CombatConfig() :
        start_delay_ms(5000),
        evade_duration_ms(300),
        line_white_threshold(500),
        ultrasonic_max_distance_cm(45),
        search_speed(150),
        attack_speed(255),
        reverse_speed(220) {}
};

#endif // COMBAT_TYPES_H
