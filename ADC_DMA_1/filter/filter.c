/*
 * filter.c
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "filter.h"

/**
 * @brief lfilter implementation
 * @input valores diferenciados
 * @output valores filtrados
 */
void intfilter(float* b, uint16_t* a, uint16_t len_b, uint16_t len_a, uint16_t* x, float* y, uint16_t len_x) {
	uint16_t* buffer = (uint16_t*)calloc(len_b, sizeof(uint16_t));

	for (int i = 0; i < len_x; i++) {
		y[i] = 0;

		for (int j = 0; j < len_b && i - j >= 0; j++) {
			y[i] += b[j] * x[i - j];
		}

		for (int j = 1; j < len_a && i - j >= 0; j++) {
			y[i] -= a[j] * y[i - j];
		}
	}

	free(buffer);
}


void floatfilter(float *b, uint16_t *a, uint16_t len_b, uint16_t len_a, float *x, float *y, uint16_t len_x) {
    float *buffer = (float *)calloc(len_b, sizeof(float));

    for (int i = 0; i < len_x; i++) {
        y[i] = 0;

        for (int j = 0; j < len_b && i - j >= 0; j++) {
            y[i] += b[j] * x[i - j];
        }

        for (int j = 1; j < len_a && i - j >= 0; j++) {
            y[i] -= a[j] * y[i - j];
        }
    }

    free(buffer);
}

