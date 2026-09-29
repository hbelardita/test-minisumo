# Minisumo Domain

Autonomous robotic combat system adhering to the 500g / 10x10cm Minisumo competition standard.

## Language

**Dohyo**:
The circular competition ring with a black playing surface and a contrasting white border.
_Avoid_: Tatami, arena, track, ring

**Border Line**:
The 2.5 cm white boundary marking the perimeter of the Dohyo.
_Avoid_: Edge, white line, tape

**Start Delay**:
The mandatory 5-second countdown after trigger activation before any motor movement is permitted.
_Avoid_: Wait time, pause, boot delay

**Start Trigger**:
The onboard momentary button pressed by the operator to arm the robot and begin the Start Delay.
_Avoid_: Power switch, reset button, starter

**Line Sensor**:
A downward-facing reflectance sensor reading analog voltage to detect the Border Line.
_Avoid_: Edge detector, floor sensor, TCRT, IR sensor

**Search Routine**:
A spin-in-place scanning behavior at controlled speed to detect the opponent using the Ultrasonic Sensor.
_Avoid_: Hunting, seeking, wandering, curved search


**Attack Routine**:
The forward drive behavior executed when the opponent is tracked within the Dohyo.
_Avoid_: Charge, ram, chase

**Evade Routine**:
The non-blocking emergency reverse and asymmetric pivot maneuver (configurable duration, default 300 ms) triggered by a Line Sensor detecting the Border Line, during which the Ultrasonic Sensor is suppressed.
_Avoid_: Backoff, escape, recovery




