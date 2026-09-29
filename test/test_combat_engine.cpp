#include <cassert>
#include <iostream>
#include "CombatTypes.h"
#include "CombatEngine.h"

void test_initial_state_is_waiting_for_start() {
    CombatEngine engine;
    assert(engine.getState() == STATE_WAITING_FOR_START);
    MotorCommand cmd = engine.getMotorCommand();
    assert(cmd.standby == true);
    assert(cmd.speed_left == 0);
    assert(cmd.speed_right == 0);
    std::cout << "[PASS] test_initial_state_is_waiting_for_start\n";
}

void test_button_press_starts_countdown() {
    CombatEngine engine;
    CombatSensors sensors = {};
    sensors.current_time_ms = 100;
    sensors.start_button_pressed = true;

    engine.update(sensors);

    assert(engine.getState() == STATE_START_DELAY);
    MotorCommand cmd = engine.getMotorCommand();
    assert(cmd.standby == true);
    assert(cmd.speed_left == 0);
    assert(cmd.speed_right == 0);
    std::cout << "[PASS] test_button_press_starts_countdown\n";
}

void test_countdown_lasts_full_duration() {
    CombatEngine engine;
    CombatSensors sensors = {};
    
    // Press button at t=1000
    sensors.current_time_ms = 1000;
    sensors.start_button_pressed = true;
    engine.update(sensors);
    assert(engine.getState() == STATE_START_DELAY);

    // Release button at t=2000, still in countdown
    sensors.start_button_pressed = false;
    sensors.current_time_ms = 2000;
    engine.update(sensors);
    assert(engine.getState() == STATE_START_DELAY);

    // At t=5999 (4999ms elapsed since start), still in countdown
    sensors.current_time_ms = 5999;
    engine.update(sensors);
    assert(engine.getState() == STATE_START_DELAY);

    // At t=6000 (5000ms elapsed), transitions to SEARCH
    sensors.current_time_ms = 6000;
    engine.update(sensors);
    assert(engine.getState() == STATE_SEARCH);
    std::cout << "[PASS] test_countdown_lasts_full_duration\n";
}

void test_search_spins_in_place_when_no_target() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors = {};

    // Trigger start countdown
    sensors.current_time_ms = 0;
    sensors.start_button_pressed = true;
    engine.update(sensors);

    // Complete 5s countdown
    sensors.current_time_ms = 5001;
    sensors.start_button_pressed = false;
    sensors.target_detected = false;
    engine.update(sensors);

    assert(engine.getState() == STATE_SEARCH);
    MotorCommand cmd = engine.getMotorCommand();
    assert(cmd.standby == false);
    // Spin in place: opposite directions
    assert(cmd.speed_left == -config.search_speed);
    assert(cmd.speed_right == config.search_speed);
    std::cout << "[PASS] test_search_spins_in_place_when_no_target\n";
}

void test_search_transitions_to_attack_when_target_detected() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors = {};

    // Complete countdown into SEARCH
    sensors.current_time_ms = 0;
    sensors.start_button_pressed = true;
    engine.update(sensors);
    sensors.current_time_ms = 5001;
    sensors.start_button_pressed = false;
    engine.update(sensors);

    // Target detected at 30 cm
    sensors.current_time_ms = 5100;
    sensors.target_detected = true;
    sensors.distance_cm = 30;
    engine.update(sensors);

    assert(engine.getState() == STATE_ATTACK);
    MotorCommand cmd = engine.getMotorCommand();
    assert(cmd.standby == false);
    assert(cmd.speed_left == config.attack_speed);
    assert(cmd.speed_right == config.attack_speed);
    std::cout << "[PASS] test_search_transitions_to_attack_when_target_detected\n";
}

void test_attack_transitions_back_to_search_when_target_lost() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors = {};

    // Reach ATTACK state
    sensors.current_time_ms = 0;
    sensors.start_button_pressed = true;
    engine.update(sensors);
    sensors.current_time_ms = 5001;
    sensors.start_button_pressed = false;
    sensors.target_detected = true;
    sensors.distance_cm = 25;
    engine.update(sensors);
    assert(engine.getState() == STATE_ATTACK);

    // Target lost
    sensors.current_time_ms = 5200;
    sensors.target_detected = false;
    sensors.distance_cm = 0;
    engine.update(sensors);

    assert(engine.getState() == STATE_SEARCH);
    MotorCommand cmd = engine.getMotorCommand();
    assert(cmd.speed_left == -config.search_speed);
    assert(cmd.speed_right == config.search_speed);
    std::cout << "[PASS] test_attack_transitions_back_to_search_when_target_lost\n";
}

void test_left_line_sensor_triggers_evade_right() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors = {};

    // Get to ATTACK state
    sensors.current_time_ms = 0;
    sensors.start_button_pressed = true;
    engine.update(sensors);
    sensors.current_time_ms = 5001;
    sensors.start_button_pressed = false;
    sensors.target_detected = true;
    sensors.line_left_raw = 800; // Black Dohyo surface
    sensors.line_right_raw = 800;
    engine.update(sensors);
    assert(engine.getState() == STATE_ATTACK);

    // Left sensor hits white border (< threshold, e.g. 200)
    sensors.current_time_ms = 5100;
    sensors.line_left_raw = 200; // White border!
    engine.update(sensors);

    assert(engine.getState() == STATE_EVADE);
    assert(engine.getEvadeDirection() == EVADE_TURN_RIGHT);
    MotorCommand cmd = engine.getMotorCommand();
    assert(cmd.standby == false);
    // Initial phase of evade: reverse
    assert(cmd.speed_left < 0 && cmd.speed_right < 0);
    std::cout << "[PASS] test_left_line_sensor_triggers_evade_right\n";
}

void test_right_line_sensor_triggers_evade_left() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors = {};

    // Get to SEARCH state
    sensors.current_time_ms = 0;
    sensors.start_button_pressed = true;
    engine.update(sensors);
    sensors.current_time_ms = 5001;
    sensors.start_button_pressed = false;
    sensors.line_left_raw = 800;
    sensors.line_right_raw = 800;
    engine.update(sensors);
    assert(engine.getState() == STATE_SEARCH);

    // Right sensor hits white border
    sensors.current_time_ms = 5200;
    sensors.line_right_raw = 150; // White border!
    engine.update(sensors);

    assert(engine.getState() == STATE_EVADE);
    assert(engine.getEvadeDirection() == EVADE_TURN_LEFT);
    std::cout << "[PASS] test_right_line_sensor_triggers_evade_left\n";
}

void test_ultrasonic_is_suppressed_during_evade() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors;

    // Start countdown
    sensors.current_time_ms = 0;
    sensors.start_button_pressed = true;
    engine.update(sensors);

    // Enter SEARCH at t=5001
    sensors.current_time_ms = 5001;
    sensors.start_button_pressed = false;
    engine.update(sensors);
    assert(engine.getState() == STATE_SEARCH);

    // Hit border line at t=5050 -> Enter EVADE
    sensors.current_time_ms = 5050;
    sensors.line_left_raw = 200; // Hit line
    sensors.line_right_raw = 800;
    engine.update(sensors);
    assert(engine.getState() == STATE_EVADE);

    // Mid-evade (t=5150), opponent appears in front of ultrasonic
    sensors.current_time_ms = 5150;
    sensors.target_detected = true;
    sensors.distance_cm = 15;
    sensors.line_left_raw = 800; // Sensors now back on black
    engine.update(sensors);

    // MUST NOT transition to ATTACK! Must stay in EVADE until evade is complete!
    assert(engine.getState() == STATE_EVADE);
    std::cout << "[PASS] test_ultrasonic_is_suppressed_during_evade\n";
}

void test_evade_returns_to_search_after_duration() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors;

    // Start countdown
    sensors.current_time_ms = 0;
    sensors.start_button_pressed = true;
    engine.update(sensors);

    // Enter SEARCH at t=5001
    sensors.current_time_ms = 5001;
    sensors.start_button_pressed = false;
    engine.update(sensors);
    assert(engine.getState() == STATE_SEARCH);

    // Trigger EVADE at t=5100 (duration is 300ms)
    sensors.current_time_ms = 5100;
    sensors.line_left_raw = 200;
    engine.update(sensors);
    assert(engine.getState() == STATE_EVADE);

    // At t=5399 (299ms into 300ms evade): still EVADE
    sensors.current_time_ms = 5399;
    sensors.line_left_raw = 800;
    engine.update(sensors);
    assert(engine.getState() == STATE_EVADE);

    // At t=5400 (300ms elapsed since 5100): returns to SEARCH
    sensors.current_time_ms = 5400;
    engine.update(sensors);
    assert(engine.getState() == STATE_SEARCH);
    std::cout << "[PASS] test_evade_returns_to_search_after_duration\n";
}

int main() {
    std::cout << "--- Running CombatEngine Tests ---\n";
    test_initial_state_is_waiting_for_start();
    test_button_press_starts_countdown();
    test_countdown_lasts_full_duration();
    test_search_spins_in_place_when_no_target();
    test_search_transitions_to_attack_when_target_detected();
    test_attack_transitions_back_to_search_when_target_lost();
    test_left_line_sensor_triggers_evade_right();
    test_right_line_sensor_triggers_evade_left();
    test_ultrasonic_is_suppressed_during_evade();
    test_evade_returns_to_search_after_duration();
    std::cout << "All tests passed successfully.\n";
    return 0;
}


