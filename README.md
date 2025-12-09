
# Embedded Closed-Loop Controller (Linux)

Proyecto de ejemplo en C que implementa un controlador en lazo cerrado sencillo, pensado para practicar separación modular, uso de headers, y polimorfismo en C a través de punteros a función.

**Índice**
- **Resumen**: descripción rápida del proyecto.
- **Requisitos**: dependencias necesarias.
- **Compilación y ejecución**: comandos útiles.
- **Estructura**: listado de carpetas y archivos.
- **Módulos**: explicación breve de `sensor`, `actuators` y `controller`.
- **Lógica de control**: comportamiento del lazo cerrado.
- **Comprobaciones**: cómo inspeccionar binarios.
- **Autor y licencia**.

**Resumen**
- **Descripción**: Programa en C que lee un sensor cada 100 ms y controla dos actuadores (LED y buzzer) según un umbral.
- **Objetivos pedagógicos**: manejo de headers, build con `make`, polimorfismo en C, y gestión de tiempos con `clock_gettime` y `clock_nanosleep`.

**Requisitos**
- **Sistema**: Linux (Ubuntu/WSL recomendado).
- **Paquetes**: instalar herramientas de compilación con:

```
sudo apt update
sudo apt install build-essential
# Opcional para compilación 32 bits:
sudo apt install gcc-multilib
```

**Compilación y ejecución**
- Compilar (usa el `Makefile`):

```
make ctl64    # genera binario de 64 bits (si está configurado en Makefile)
# make ctl32  # opcional: genera binario de 32 bits si Makefile lo soporta
```

- Limpiar artefactos:

```
make clean
```

- Ejecutar (umbral opcional):

```
./ctl64         # umbral por defecto (p. ej. 0.5)
./ctl64 0.7     # umbral explícito
```

**Estructura del repositorio**
- `Makefile`: reglas de compilación.
- `ctl64` (posible binario generado).
- `controller/ctl.c`: código del controlador.
- `sensor/`:
  - `sensor.h` y `sensor.c` — interfaz y simulador de sensor.
- `actuators/`:
  - `actuator.h`, `led_actuator.c`, `buzzer_actuator.c` — interfaz genérica y backends.

**Módulos (breve)**
- **Sensor (`sensor/`)**: `sensor_init()` y `sensor_read()`; la implementación actual genera lecturas aleatorias en [0,1].
- **Actuadores (`actuators/`)**: interfaz `actuator_t` con punteros `activate`, `deactivate`, `status`; backends para LED y buzzer.
- **Controlador (`controller/ctl.c`)**: bucle periódico (100 ms), lectura, lógica de umbral, y manejo de deadlines para apagado diferido.

**Lógica de control**
- Si `sensor >= threshold`:
  - Enciende LED y buzzer inmediatamente.
  - Cancela timers de apagado pendientes.
- Si `sensor < threshold` y antes estábamos por encima:
  - Programa apagado del buzzer a t + 1 s.
  - Programa apagado del LED a t + 5 s.
- El tiempo se maneja con `clock_gettime(CLOCK_MONOTONIC, ...)` y comparaciones entre `struct timespec`.

**Salida / Logs**
- El programa imprime una línea CSV por ciclo con formato: `t[s], sensor, led, buzzer`.
- Ejemplo:

```
0.100, 0.123, 0, 0
0.200, 0.812, 1, 1
```

**Inspección de binarios**
- Para verificar arquitectura y cabecera ELF:

```
file ctl64
readelf -h ctl64
```

Si compilas versión 32 bits, sustituye `ctl64` por `ctl32`.

**Autor y licencia**
- **Autor**: juanAG1210 (repositorio `segundatarealinux`).


---
