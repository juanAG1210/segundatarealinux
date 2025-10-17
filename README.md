
## SEGUND_TAREA_LINUX

```markdown
# Embedded Systems Programming – Homework

Este repositorio implementa:
1. **Librería de sensor** (`sensor/`)
2. **Interfaz de actuadores polimórfica** con LED y buzzer (`actuators/`)
3. **Controlador en lazo cerrado** (`controller/ctl.c`) que toma muestras cada 100 ms, compara con umbral y (des)activa actuadores con temporizadores monotónicos.

## Estructura
```

embedded-session-hw/
├── Makefile
├── README.md
├── ai_log.md
├── sensor/
│   ├── sensor.h
│   └── sensor.c
├── actuators/
│   ├── actuator.h
│   ├── led_actuator.c
│   └── buzzer_actuator.c
├── controller/
│   └── ctl.c
└── sensor_feed.csv

````

## Compilación
Requisitos (Linux): `gcc` y (opcional para 32-bit) `gcc-multilib`.

```bash
make ctl64    # binario 64-bit
./ctl64       # ejecutar

make ctl32    # binario 32-bit (requiere multilib)
./ctl32
````

## Parámetros por defecto

* Umbral: `70.0`
* Período de muestreo: `100 ms`
* Apagado diferido: buzzer `1 s`, LED `5 s`
* Fuente del sensor: aleatorio 0..100. Si existe `sensor_feed.csv` con una columna de valores (double), se usa el CSV y se reinicia al llegar al final.

## Ejemplo de salida

```
[1.234] x=18.00 LED=0 BZ=0
[1.334] x=75.00 LED=1 BZ=1
[1.434] x=82.00 LED=1 BZ=1
[2.534] x=20.00 LED=1 BZ=0
[6.534] x=10.00 LED=0 BZ=0
```

## Inspección de binarios

```bash
file ctl64
# Ej.: ELF 64-bit LSB executable, x86-64

readelf -h ctl64 | sed -n '1,12p'
# Revisa: Class: ELF64, Machine, Entry point

file ctl32
# Ej.: ELF 32-bit LSB executable, Intel 80386

readelf -h ctl32 | sed -n '1,12p'
# Revisa: Class: ELF32
```

## Conceptos clave

* **Compilación**: cada `.c` → `.o` (síntaxis, tipos).
* **Enlazado (link)**: une `.o` en un ejecutable; falla si hay símbolos múltiples o faltantes.
* **Headers con guards**: solo **declaraciones**; evitan redefiniciones cuando se incluyen varias veces.
* **`extern`**: se usaría si hubiera variables globales compartidas; aquí no fue necesario (diseño sin globals públicos).
* **Polimorfismo en C**: punteros a función en `Actuator` permiten cambiar backend (LED o buzzer) sin tocar el controlador.
* **Tiempo monotónico**: `CLOCK_MONOTONIC` no retrocede si cambia la hora del sistema.

## Notas de calidad

* Código con `-Wall -Wextra -std=c11`.
* Estructura separada por módulos.
* `Makefile` con `ctl64`, `ctl32`, `clean`.

## Créditos / Uso de IA

Consulta `ai_log.md` con los prompts/respuestas usados y comentarios breves de por qué se hicieron.

````

---

## ai_log.md (root)
> Rellena con tus prompts y respuestas. Incluyo el esqueleto inicial con esta conversación.
```markdown
# Bitácora de interacción con IA

> **Formato sugerido por entrada**
> - **Motivación del prompt**: (¿Qué necesitaba lograr o aclarar?)
> - **Prompt enviado**
> - **Respuesta recibida (resumen)**
> - **Cómo refiné el siguiente prompt**

---

## 1) Visión general del trabajo
- **Motivación**: No entendía el alcance ni el plan de trabajo.
- **Prompt**: “pudes explicarme este trabajo en español no se que hacer”
- **Respuesta (resumen)**: Plan en pasos, módulos requeridos, mini‑ejemplos, Makefile base.
- **Refinamiento**: Pedí los archivos completos listos para copiar.

## 2) Generación del repositorio completo
- **Motivación**: Tener todos los archivos listos con guards y `clock_gettime`.
- **Prompt**: “ok si puedes hacerlo”
- **Respuesta (resumen)**: Se entregó estructura final con todo el código, README y Makefile.
- **Refinamiento**: (Aquí agrega futuros cambios que pidas: lectura desde CSV, cambiar umbral, etc.)
````

---

## sensor/sensor.h

```c
#ifndef SENSOR_H
#define SENSOR_H

#include <stddef.h>

// Inicializa la fuente del sensor (semilla rand o abre CSV si existe)
void   sensor_init(void);

// Lee un valor de sensor como double
// Si hay CSV, lee siguiente valor; al llegar a EOF, reinicia.
// Si no hay CSV, retorna un aleatorio en [0,100].
double sensor_read(void);

#endif // SENSOR_H
```

---

## sensor/sensor.c

```c
#include "sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static FILE *g_csv = NULL;

static int file_exists(const char *path){
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    fclose(f);
    return 1;
}

void sensor_init(void) {
    // Opción CSV si existe el archivo
    if (file_exists("sensor_feed.csv")) {
        g_csv = fopen("sensor_feed.csv", "r");
    } else {
        g_csv = NULL;
    }
    // Semilla para aleatorio como respaldo
    srand((unsigned)time(NULL));
}

static double read_csv_or_restart(void){
    if (!g_csv) return 0.0; // no CSV
    double x = 0.0;
    int rc = fscanf(g_csv, "%lf", &x);
    if (rc == 1) return x;
    // Reiniciar si EOF o error
    clearerr(g_csv);
    fseek(g_csv, 0, SEEK_SET);
    if (fscanf(g_csv, "%lf", &x) == 1) return x;
    return 0.0; // si el csv está vacío
}

double sensor_read(void) {
    if (g_csv) {
        return read_csv_or_restart();
    }
    // Aleatorio 0..100 si no hay CSV
    return (double)(rand() % 101);
}
```

---

## actuators/actuator.h

```c
#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <stdbool.h>

typedef struct Actuator {
    void *params;
    void (*activate)(struct Actuator *);
    void (*deactivate)(struct Actuator *);
    bool (*status)(struct Actuator *);
} Actuator;

// Constructores de backends
Actuator make_led_actuator(void);
Actuator make_buzzer_actuator(void);

#endif // ACTUATOR_H
```

---

## actuators/led_actuator.c

```c
#include "actuator.h"
#include <stdlib.h>

typedef struct { bool on; } LedParams;

static void led_on(Actuator *a){
    ((LedParams*)a->params)->on = true;  // aquí iría GPIO high
}
static void led_off(Actuator *a){
    ((LedParams*)a->params)->on = false; // aquí iría GPIO low
}
static bool led_status(Actuator *a){
    return ((LedParams*)a->params)->on;
}

Actuator make_led_actuator(void){
    Actuator a = {0};
    a.params = malloc(sizeof(LedParams));
    ((LedParams*)a.params)->on = false;
    a.activate   = led_on;
    a.deactivate = led_off;
    a.status     = led_status;
    return a;
}
```

---

## actuators/buzzer_actuator.c

```c
#include "actuator.h"
#include <stdlib.h>

typedef struct { bool on; } BzParams;

static void bz_on(Actuator *a){
    ((BzParams*)a->params)->on = true;   // aquí iría PWM/enable
}
static void bz_off(Actuator *a){
    ((BzParams*)a->params)->on = false;  // aquí iría disable
}
static bool bz_status(Actuator *a){
    return ((BzParams*)a->params)->on;
}

Actuator make_buzzer_actuator(void){
    Actuator a = {0};
    a.params = malloc(sizeof(BzParams));
    ((BzParams*)a.params)->on = false;
    a.activate   = bz_on;
    a.deactivate = bz_off;
    a.status     = bz_status;
    return a;
}
```

---

## controller/ctl.c

```c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include "../sensor/sensor.h"
#include "../actuators/actuator.h"

static double now_s(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec/1e9;
}

int main(void){
    const double TH = 70.0;          // umbral
    const double T_SAMPLE = 0.100;   // 100 ms
    const double OFF_BZ = 1.0;       // 1 s (buzzer)
    const double OFF_LED = 5.0;      // 5 s (LED)

    sensor_init();
    Actuator led = make_led_actuator();
    Actuator bz  = make_buzzer_actuator();

    double t_next = now_s();
    double t_deadline_bz = -1.0, t_deadline_led = -1.0;

    for(;;){
        // Espera pasiva simple con precisión aceptable para demo
        double t = now_s();
        if (t < t_next){ usleep(1000); continue; } // 1 ms granulado
        t_next += T_SAMPLE;

        double x = sensor_read();

        if (x >= TH){
            led.activate(&led);
            bz.activate(&bz);
            t_deadline_bz = -1.0;  // cancelar timers
            t_deadline_led = -1.0;
        } else {
            // Programar apagados diferidos
            t_deadline_bz  = t + OFF_BZ;
            t_deadline_led = t + OFF_LED;
        }

        // Chequear vencimientos
        t = now_s();
        if (t_deadline_bz > 0 && t >= t_deadline_bz){
            bz.deactivate(&bz);
            t_deadline_bz = -1.0;
        }
        if (t_deadline_led > 0 && t >= t_deadline_led){
            led.deactivate(&led);
            t_deadline_led = -1.0;
        }

        // Log
        printf("[%.3f] x=%.2f LED=%d BZ=%d\n", t, x, led.status(&led), bz.status(&bz));
        fflush(stdout);
    }
    return 0;
}
```

---

## sensor_feed.csv (opcional)

```text
10
20
30
80
85
60
50
90
40
15
```

---

## tests/ (opcional)

*(Deja vacío o agrega pruebas más adelante; no es obligatorio para compilar.)*

```text
(placeholder)
```

---

## Nota rápida sobre errores comunes

* **Redefiniciones**: no pongas definiciones en headers; usa guards `#ifndef/#define/#endif`.
* **Reloj**: usa `CLOCK_MONOTONIC` para evitar saltos si cambia la hora del sistema.
* **32 bits**: si falla `ctl32`, instala `gcc-multilib` u otras libs i386 según tu distro.

```
```

