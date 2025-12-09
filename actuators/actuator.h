#ifndef ACTUATOR_H
#define ACTUATOR_H

/* Interfaz genérica de actuador (polimórfica vía punteros a función) */

typedef struct actuator actuator_t;

struct actuator {
    void *params;  /* Datos específicos del backend (LED, buzzer, etc.) */

    void (*activate)(actuator_t *self);
    void (*deactivate)(actuator_t *self);
    int  (*status)(const actuator_t *self);  /* 0 = off, !=0 = on */
};

/* Factories / init functions para cada backend */
void led_actuator_init(actuator_t *a, const char *name);
void buzzer_actuator_init(actuator_t *a, const char *name);

#endif /* ACTUATOR_H */
