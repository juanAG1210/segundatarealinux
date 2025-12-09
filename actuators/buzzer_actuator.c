#include "actuator.h"

#include <stdio.h>
#include <stdlib.h>

struct buzzer_params {
    const char *name;
    int is_on;
};

static void buzzer_activate(actuator_t *self)
{
    struct buzzer_params *p = (struct buzzer_params *)self->params;
    if (!p->is_on) {
        p->is_on = 1;
        printf("[BUZZER %s] ON\n", p->name);
    }
}

static void buzzer_deactivate(actuator_t *self)
{
    struct buzzer_params *p = (struct buzzer_params *)self->params;
    if (p->is_on) {
        p->is_on = 0;
        printf("[BUZZER %s] OFF\n", p->name);
    }
}

static int buzzer_status(const actuator_t *self)
{
    const struct buzzer_params *p = (const struct buzzer_params *)self->params;
    return p->is_on;
}

void buzzer_actuator_init(actuator_t *a, const char *name)
{
    struct buzzer_params *p = (struct buzzer_params *)malloc(sizeof(*p));
    if (!p) {
        fprintf(stderr, "buzzer_actuator_init: malloc failed\n");
        exit(EXIT_FAILURE);
    }

    p->name = name;
    p->is_on = 0;

    a->params     = p;
    a->activate   = buzzer_activate;
    a->deactivate = buzzer_deactivate;
    a->status     = buzzer_status;
}
