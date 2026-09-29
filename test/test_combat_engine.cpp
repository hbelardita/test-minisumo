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

int main() {
    std::cout << "--- Running CombatEngine Tests ---\n";
    test_initial_state_is_waiting_for_start();
    test_button_press_starts_countdown();
    test_countdown_lasts_full_duration();
    test_search_spins_in_place_when_no_target();
    test_search_transitions_to_attack_when_target_detected();
    test_attack_transitions_back_to_search_when_target_lost();
    std::cout << "All tests passed successfully.\n";
    return 0;
}

