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

int main() {
    std::cout << "--- Running CombatEngine Slice 1 Tests ---\n";
    test_initial_state_is_waiting_for_start();
    test_button_press_starts_countdown();
    test_countdown_lasts_full_duration();
    std::cout << "All Slice 1 tests passed successfully.\n";
    return 0;
}
