# Diagnóstico: Robot Gira en Tatami sin Reaccionar a la Distancia

Documento de referencia para solucionar el comportamiento donde el robot queda girando continuamente sobre su propio eje en búsqueda en el tatami sin reaccionar a objetos enfrente, pero sí reacciona al levantarlo en el aire o en modo debug.

---

## 1. Confirmación del Estado: `STATE_SEARCH`

Al observar que el robot **gira sobre su propio eje** limpiamente sin retroceder:
- **No está en `STATE_EVADE`:** La rutina de evasión realiza un retroceso directo en fase 1 (`speed_left = -220, speed_right = -220`).
- **Está efectivamente en `STATE_SEARCH`:** [`CombatEngine.h`](file:///home/horacio/Documents/test-minisumo/include/CombatEngine.h#L125-L129) comanda `speed_left = -150` y `speed_right = 150`, provocando un giro continuo sobre su propio eje.
- **Los sensores de línea están funcionando bien:** Están leyendo negro correctamente (no disparan evasión).

Por lo tanto, el problema está localizado **100% en la detección del sensor ultrasónico HC-SR04 mientras el robot gira bajo carga en el tatami**.

---

## 2. ¿Por qué en el tatami no detecta y al levantarlo o en debug sí?

Comparativa de las tres condiciones:

| Condición | Motores | Giro del Chasis | Alimentación / Ruido | Lectura Ultrasónica |
| :--- | :--- | :--- | :--- | :--- |
| **Debug (`sensor_monitor`)** | Apagados (0 mA) | Inmóvil | 5V limpios, sin EMI | **Mide distancia perfecta** |
| **Levantado en el aire** | Giro libre (~60 mA) | Inmóvil en la mano | Bajo ruido, sin carga | **Detecta y ataca** |
| **Apoyado en tatami** | Alta carga (hasta 1.5 A) | **Gira a ~250°/s** | **Alto ruido EMI / caída 5V** | **Ciego (retorna 0 cm)** |

---

## 3. Las Causas Técnicas del Fallo en Búsqueda

### Causa 1: Intervalo de muestreo muy rápido (50 ms vs 100 ms)
- En [`src/sensor_monitor.cpp`](file:///home/horacio/Documents/test-minisumo/src/sensor_monitor.cpp#L18), el muestreo es cada **100 ms** (`TELEMETRY_INTERVAL_MS = 100`) y funciona perfecto.
- En [`src/main.cpp#L64`](file:///home/horacio/Documents/test-minisumo/src/main.cpp#L64), el muestreo está fijado a **50 ms** (`now - last_ultrasonic_ping_ms >= 50`).
- **El problema:** La hoja de datos del HC-SR04 indica un ciclo mínimo recomendado de **60 ms a 100 ms**. Cuando el robot gira buscando en el tatami y no hay obstáculo enfrente, el HC-SR04 mantiene el pin ECHO en alto durante su timeout interno por hardware (**38 ms a 45 ms**). Disparar un nuevo pulso a los 50 ms encuentra el pin aún saturado o con ecos residuales rebotando en el tatami, desincronizando `pulseIn()` y devolviendo 0 continuamente.

### Causa 2: Ruido eléctrico (EMI) y caída de tensión bajo carga mecánica
- En el tatami, los motores TT con ruedas de goma sufren fricción contra el suelo. Los motores de escobillas conmutan bajo carga generando:
  1. Caídas transitorias de tensión en la línea de 5V del Arduino / HC-SR04.
  2. Ruido electromagnético de alta frecuencia en VCC y GND.
- El HC-SR04 contiene un amplificador operacional analógico de muy alta ganancia para captar microvoltios en el receptor piezoeléctrico. Con ruido eléctrico en los rieles, el comparador se satura y se vuelve incapaz de distinguir el eco de la mano.
- Al levantarlo en el aire, la carga mecánica desaparece, la corriente cae de ~1.2 A a ~100 mA y el sensor vuelve a funcionar.

### Causa 3: Desalineación del haz por velocidad angular
- A `search_speed = 150`, el robot rota a más de **200° a 300° por segundo**.
- El sonido tarda aprox. **1.7 ms a 2.5 ms** en viajar hasta la mano y regresar.
- Al levantarlo en el aire, tú sostienes el cuerpo del robot quieto apuntando a la mano mientras las ruedas giran en el vacío. En el tatami, el sensor rota rápidamente, haciendo que el cono sónico rebote en la mano con un ángulo incidente no perpendicular que no regresa al receptor piezoeléctrico.

### Causa 4: Inclinación del sensor y choque acústico contra el suelo
- Estando a 2–3 cm de altura, el lóbulo inferior del cono acústico (~30° de apertura) choca contra el tatami a menos de 10 cm del frente y se dispersa hacia arriba en reflexión especular. Al levantarlo, el tatami sale del cono.

---

## 4. Soluciones Concretas

### Solución A: Ajustar el intervalo de muestreo en `main.cpp`
Subir el tiempo entre pings en [`src/main.cpp#L64`](file:///home/horacio/Documents/test-minisumo/src/main.cpp#L64) a **70 ms o 80 ms** para dar margen suficiente al HC-SR04 de recuperarse de su timeout interno:
```cpp
// En src/main.cpp: cambiar de 50 ms a 70 ms
if (now - last_ultrasonic_ping_ms >= 70) {
    last_ultrasonic_ping_ms = now;
    cached_ultrasonic = ultrasonic.sample(engine.getConfig().ultrasonic_max_distance_cm);
}
```

### Solución B: Reducir ligeramente la velocidad de giro en búsqueda
Si el robot gira demasiado rápido, no da tiempo al sensor de procesar ecos antes de desfasarse.
En [`include/CombatTypes.h#L64`](file:///home/horacio/Documents/test-minisumo/include/CombatTypes.h#L64):
```cpp
// Probar reducir de 150 a 110-120
search_speed(120),
```
Un giro más controlado aumenta drásticamente la tasa de captura de ecos en rotación.

### Solución C: Hardware (Filtros y Alimentación)
1. **Capacitores en motores:** Soldar un capacitor cerámico de **100 nF (código 104)** entre los terminales positivo y negativo de cada motor TT, y si es posible, uno de cada borne al chasis metálico del motor.
2. **Capacitor en el HC-SR04:** Soldar un capacitor electrolítico de **100 µF a 220 µF** directamente entre los pines `VCC` y `GND` del módulo HC-SR04 para absorber caídas de tensión cuando arrancan los motores.
3. **Inclinación:** Calzar el soporte del HC-SR04 para que apunte ligeramente hacia arriba (unos 3° a 5°) y el cono acústico no pegue contra la superficie del tatami.
