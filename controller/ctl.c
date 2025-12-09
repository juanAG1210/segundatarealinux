#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sensor.h"
#include "actuator.h"

/* Umbral por defecto; se puede sobrescribir con argv[1] */
static double g_threshold = 0.5;

/* Diferencia en segundos entre dos timespec */
static double timespec_diff_s(const struct timespec *start,
                              const struct timespec *end)
{
    time_t ds = end->tv_sec - start->tv_sec;
    long   dn = end->tv_nsec - start->tv_nsec;
    return (double)ds + (double)dn / 1e9;
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
        g_threshold = atof(argv[1]);
    }

    /* Inicializar sensor y actuadores */
    sensor_init();

    actuator_t led;
    actuator_t buzzer;

    led_actuator_init(&led, "STATUS");
    buzzer_actuator_init(&buzzer, "ALARM");

    /* Tiempo base (t = 0 en CLOCK_MONOTONIC) */
    struct timespec t0;
    if (clock_gettime(CLOCK_MONOTONIC, &t0) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }

    /* Timers de apagado (en segundos relativos a t0) */
    double led_off_deadline    = 0.0; /* 0.0 → sin timer pendiente */
    double buzzer_off_deadline = 0.0;

    int was_above = 0;

    /* Intervalo de muestreo: 100 ms */
    const struct timespec interval = { .tv_sec = 0, .tv_nsec = 100 * 1000 * 1000 };

    /* Cabecera del log: tiempo, sensor, led, buzzer */
    printf("# t[s], sensor, led, buzzer\n");
    fflush(stdout);

    while (1) {
        /* Dormimos exactamente 100 ms en tiempo monotónico */
        if (clock_nanosleep(CLOCK_MONOTONIC, 0, &interval, NULL) != 0) {
            perror("clock_nanosleep");
            return EXIT_FAILURE;
        }

        struct timespec now_ts;
        if (clock_gettime(CLOCK_MONOTONIC, &now_ts) != 0) {
            perror("clock_gettime");
            return EXIT_FAILURE;
        }
        double t = timespec_diff_s(&t0, &now_ts);

        /* Leer sensor */
        double value = sensor_read();
        int above = (value >= g_threshold);

        /* Lógica de control */
        if (above) {
            /* Activar inmediatamente y cancelar timers */
            led.activate(&led);
            buzzer.activate(&buzzer);

            led_off_deadline = 0.0;
            buzzer_off_deadline = 0.0;
        } else {
            /* Transición de "por encima" → "por debajo" del umbral */
            if (was_above) {
                /* Programar apagados relativos al instante actual */
                if (buzzer.status(&buzzer)) {
                    buzzer_off_deadline = t + 1.0;  /* 1 s */
                }
                if (led.status(&led)) {
                    led_off_deadline = t + 5.0;     /* 5 s */
                }
            }
        }

        /* Gestionar timers de apagado */
        if (buzzer_off_deadline > 0.0 && t >= buzzer_off_deadline) {
            buzzer.deactivate(&buzzer);
            buzzer_off_deadline = 0.0;
        }

        if (led_off_deadline > 0.0 && t >= led_off_deadline) {
            led.deactivate(&led);
            led_off_deadline = 0.0;
        }

        was_above = above;

        /* Log de estados */
        int led_state    = led.status(&led);
        int buzzer_state = buzzer.status(&buzzer);

        printf("%.3f, %.3f, %d, %d\n", t, value, led_state, buzzer_state);
        fflush(stdout);
    }

    return 0;
}
