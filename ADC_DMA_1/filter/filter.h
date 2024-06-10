/*
 * filter.h
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

void intfilter(float * b, uint16_t* a, uint16_t len_b, uint16_t len_a, uint16_t* x, float* y, uint16_t len_x);
void floatfilter(float *b, uint16_t *a, uint16_t len_b, uint16_t len_a, float *x, float *y, uint16_t len_x);

#endif /* FILTER_H_ */
