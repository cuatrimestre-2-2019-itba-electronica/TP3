/*
 * FSK.h
 *
 *  Created on: Oct 25, 2019
 *      Author: gonzalosilva
 */

#ifndef FSK_H_
#define FSK_H_

#include <stdbool.h>
#include <stdint.h>

#define AMOUNT_POINTS 50
#define SAMPLE_FREQUENCY_1 (AMOUNT_POINTS * 1200)
#define SAMPLE_FREQUENCY_0 (int)(1.833333 * SAMPLE_FREQUENCY_1)
#define FSK_BUFFER_LEN 40000

typedef void (* FSK_callback)(uint16_t sample);

/*
 * Inicializacion de FSK.
 * cb: Funcion que se llama cuando se debe generar cada muestra.
 * Recibe valores entre 0x0000 (minimo) y 0x0FFF (maximo)
 */
void FSK_init(FSK_callback cb);


bool FSK_push_to_buffer (bool num2pop); //si devuelve un 1 es que se pudo popear

#endif /* FSK_H_ */
