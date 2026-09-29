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
    assert(engine.isLedActive() == false);
    std::cout << "[PASS] test_initial_state_is_waiting_for_start\n";
}

void test_button_press_starts_countdown() {
    CombatEngine engine;
    CombatSensors sensors;
    sensors.current_time_ms = 100;
    sensors.start_trigger_active = true;

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
    CombatSensors sensors;
    
    // Press trigger at t=1000
    sensors.current_time_ms = 1000;
    sensors.start_trigger_active = true;
    engine.update(sensors);
    assert(engine.getState() == STATE_START_DELAY);

    // Release trigger at t=2000, still in countdown
    sensors.start_trigger_active = false;
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
    CombatSensors sensors;

    // Trigger start countdown
    sensors.current_time_ms = 0;
    sensors.start_trigger_active = true;
    engine.update(sensors);

    // Complete 5s countdown
    sensors.current_time_ms = 5001;
    sensors.start_trigger_active = false;
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
    CombatSensors sensors;

    // Complete countdown into SEARCH
    sensors.current_time_ms = 0;
    sensors.start_trigger_active = true;
    engine.update(sensors);
    sensors.current_time_ms = 5001;
    sensors.start_trigger_active = false;
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
    CombatSensors sensors;

    // Reach ATTACK state
    sensors.current_time_ms = 0;
    sensors.start_trigger_active = true;
    engine.update(sensors);
    sensors.current_time_ms = 5001;
    sensors.start_trigger_active = false;
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

void test_direct_attack_when_target_detected_at_countdown_end() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors;

    // Start countdown
    sensors.current_time_ms = 0;
    sensors.start_trigger_active = true;
    engine.update(sensors);

    // Right when countdown completes, opponent is directly ahead!
    sensors.current_time_ms = 5001;
    sensors.start_trigger_active = false;
    sensors.target_detected = true;
    sensors.distance_cm = 20;
    engine.update(sensors);

    // Must jump directly into ATTACK rather than spinning away in SEARCH
    assert(engine.getState() == STATE_ATTACK);
    MotorCommand cmd = engine.getMotorCommand();
    assert(cmd.standby == false);
    assert(cmd.speed_left == config.attack_speed);
    assert(cmd.speed_right == config.attack_speed);
    std::cout << "[PASS] test_direct_attack_when_target_detected_at_countdown_end\n";
}

void test_left_line_sensor_triggers_evade_right() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors;

    // Get to ATTACK state
    sensors.current_time_ms = 0;
    sensors.start_trigger_active = true;
    engine.update(sensors);
    sensors.current_time_ms = 5001;
    sensors.start_trigger_active = false;
    sensors.target_detected = true;
    sensors.line_left_raw = 800; // Black Dohyo surface
    sensors.line_right_raw = 800;
    engine.update(sensors);
    assert(engine.getState() == STATE_ATTACK);

    // Left sensor hits Border Line (< threshold 500)
    sensors.current_time_ms = 5100;
    sensors.line_left_raw = 200; // White border!
    engine.update(sensors);

    assert(engine.getState() == STATE_EVADE);
    assert(engine.getEvadeDirection() == EVADE_TURN_RIGHT);
    MotorCommand cmd = engine.getMotorCommand();
    assert(cmd.standby == false);
    // Initial Phase 1 of evade: reverse backwards away from edge
    assert(cmd.speed_left == -config.reverse_speed);
    assert(cmd.speed_right == -config.reverse_speed);
    std::cout << "[PASS] test_left_line_sensor_triggers_evade_right\n";
}

void test_right_line_sensor_triggers_evade_left() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors;

    // Get to SEARCH state
    sensors.current_time_ms = 0;
    sensors.start_trigger_active = true;
    engine.update(sensors);
    sensors.current_time_ms = 5001;
    sensors.start_trigger_active = false;
    sensors.line_left_raw = 800;
    sensors.line_right_raw = 800;
    engine.update(sensors);
    assert(engine.getState() == STATE_SEARCH);

    // Right sensor hits Border Line
    sensors.current_time_ms = 5200;
    sensors.line_right_raw = 150; // White border!
    engine.update(sensors);

    assert(engine.getState() == STATE_EVADE);
    assert(engine.getEvadeDirection() == EVADE_TURN_LEFT);
    std::cout << "[PASS] test_right_line_sensor_triggers_evade_left\n";
}

void test_both_line_sensors_trigger_evade_full_turn() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors;

    // Get to SEARCH state
    sensors.current_time_ms = 0;
    sensors.start_trigger_active = true;
    engine.update(sensors);
    sensors.current_time_ms = 5001;
    sensors.start_trigger_active = false;
    engine.update(sensors);
    assert(engine.getState() == STATE_SEARCH);

    // Both sensors hit Border Line simultaneously (head-on edge collision)
    sensors.current_time_ms = 5200;
    sensors.line_left_raw = 180;
    sensors.line_right_raw = 190;
    engine.update(sensors);

    assert(engine.getState() == STATE_EVADE);
    assert(engine.getEvadeDirection() == EVADE_FULL_TURN);
    std::cout << "[PASS] test_both_line_sensors_trigger_evade_full_turn\n";
}

void test_evade_phase_two_asymmetric_turn_motor_commands() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors;

    // Enter SEARCH at t=5001
    sensors.current_time_ms = 0;
    sensors.start_trigger_active = true;
    engine.update(sensors);
    sensors.current_time_ms = 5001;
    sensors.start_trigger_active = false;
    engine.update(sensors);

    // Hit left border at t=5100 (duration=300ms, half=150ms)
    sensors.current_time_ms = 5100;
    sensors.line_left_raw = 200;
    engine.update(sensors);
    assert(engine.getState() == STATE_EVADE);
    assert(engine.getEvadeDirection() == EVADE_TURN_RIGHT);

    // Phase 1 (t=5150, < 150ms elapsed): both reverse
    sensors.current_time_ms = 5150;
    sensors.line_left_raw = 800; // Sensor back on black
    engine.update(sensors);
    MotorCommand cmd1 = engine.getMotorCommand();
    assert(cmd1.speed_left == -config.reverse_speed);
    assert(cmd1.speed_right == -config.reverse_speed);

    // Phase 2 (t=5260, >= 150ms elapsed): asymmetric spin away from left edge (turns right)
    sensors.current_time_ms = 5260;
    engine.update(sensors);
    MotorCommand cmd2 = engine.getMotorCommand();
    assert(cmd2.speed_left == config.search_speed);
    assert(cmd2.speed_right == -config.search_speed);
    std::cout << "[PASS] test_evade_phase_two_asymmetric_turn_motor_commands\n";
}

void test_ultrasonic_is_suppressed_during_evade() {
    CombatConfig config;
    CombatEngine engine(config);
    CombatSensors sensors;

    // Start countdown
    sensors.current_time_ms = 0;
    sensors.start_trigger_active = true;
    engine.update(sensors);

    // Enter SEARCH at t=5001
    sensors.current_time_ms = 5001;
    sensors.start_trigger_active = false;
    engine.update(sensors);
    assert(engine.getState() == STATE_SEARCH);

    // Hit Border Line at t=5050 -> Enter EVADE
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
    sensors.start_trigger_active = true;
    engine.update(sensors);

    // Enter SEARCH at t=5001
    sensors.current_time_ms = 5001;
    sensors.start_trigger_active = false;
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

void test_led_blinks_in_countdown_and_solid_in_combat() {
    CombatEngine engine;
    CombatSensors sensors;

    // Start countdown at t=100
    sensors.current_time_ms = 100;
    sensors.start_trigger_active = true;
    engine.update(sensors);
    assert(engine.getState() == STATE_START_DELAY);

    // LED should be active at t=100 (100 / 250 = 0 -> even)
    assert(engine.isLedActive() == true);

    // At t=350 (350 / 250 = 1 -> odd): blinking off
    sensors.current_time_ms = 350;
    sensors.start_trigger_active = false;
    engine.update(sensors);
    assert(engine.isLedActive() == false);

    // At t=5100 (countdown finished -> SEARCH): LED solid ON
    sensors.current_time_ms = 5101;
    engine.update(sensors);
    assert(engine.getState() == STATE_SEARCH);
    assert(engine.isLedActive() == true);
    std::cout << "[PASS] test_led_blinks_in_countdown_and_solid_in_combat\n";
}

int main() {
    std::cout << "--- Running Complete CombatEngine Test Suite ---\n";
    test_initial_state_is_waiting_for_start();
    test_button_press_starts_countdown();
    test_countdown_lasts_full_duration();
    test_search_spins_in_place_when_no_target();
    test_search_transitions_to_attack_when_target_detected();
    test_attack_transitions_back_to_search_when_target_lost();
    test_direct_attack_when_target_detected_at_countdown_end();
    test_left_line_sensor_triggers_evade_right();
    test_right_line_sensor_triggers_evade_left();
    test_both_line_sensors_trigger_evade_full_turn();
    test_evade_phase_two_asymmetric_turn_motor_commands();
    test_ultrasonic_is_suppressed_during_evade();
    test_evade_returns_to_search_after_duration();
    test_led_blinks_in_countdown_and_solid_in_combat();
    std::cout << "All 14 tests passed successfully.\n";
    return 0;
}
