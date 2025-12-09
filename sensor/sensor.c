#include "sensor.h"

#include <stdlib.h>
#include <time.h>

/* Implementación muy simple:
 * - sensor_init() inicializa la semilla del RNG.
 * - sensor_read() retorna un double uniforme entre 0.0 y 1.0.
 */

void sensor_init(void)
{
    /* Semilla basada en tiempo; solo hace falta una vez,
       pero si se llamara más veces no es grave. */
    srand((unsigned)time(NULL));
}

double sensor_read(void)
{
    /* RAND_MAX es entero, convertimos a double para tener [0,1] */
    double value = (double)rand() / (double)RAND_MAX;
    return value;
}
