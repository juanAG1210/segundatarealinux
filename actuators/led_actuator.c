#include "actuator.h"

#include <stdio.h>
#include <stdlib.h>

struct led_params {
    const char *name;
    int is_on;
};

static void led_activate(actuator_t *self)
{
    struct led_params *p = (struct led_params *)self->params;
    if (!p->is_on) {
        p->is_on = 1;
        printf("[LED %s] ON\n", p->name);
    }
}

static void led_deactivate(actuator_t *self)
{
    struct led_params *p = (struct led_params *)self->params;
    if (p->is_on) {
        p->is_on = 0;
        printf("[LED %s] OFF\n", p->name);
    }
}

static int led_status(const actuator_t *self)
{
    const struct led_params *p = (const struct led_params *)self->params;
    return p->is_on;
}

void led_actuator_init(actuator_t *a, const char *name)
{
    struct led_params *p = (struct led_params *)malloc(sizeof(*p));
    if (!p) {
        fprintf(stderr, "led_actuator_init: malloc failed\n");
        exit(EXIT_FAILURE);
    }

    p->name = name;
    p->is_on = 0;

    a->params     = p;
    a->activate   = led_activate;
    a->deactivate = led_deactivate;
    a->status     = led_status;
}
