#ifndef SENSOR_H
#define SENSOR_H

/* Simple sensor interface
 * sensor_init()  → inicializa la fuente de datos
 * sensor_read()  → devuelve una lectura en double (0.0 – 1.0 aprox.)
 *
 * En esta implementación el sensor genera valores aleatorios.
 * (Cumple con el enunciado: el sensor puede ser random o desde CSV).
 */

void sensor_init(void);
double sensor_read(void);

#endif /* SENSOR_H */
