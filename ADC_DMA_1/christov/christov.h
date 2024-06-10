/*
 * christov.h
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#ifndef CHRISTOV_H_
#define CHRISTOV_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mathematics.h"
#include "prefiltering.h"

float* christov_differentiation(float *input, int length);
float* chistov_noise(float *diff_signal, uint16_t total_taps, int length);
int* christov(uint16_t* mock_input, float* MA3, int length, int fs, int  *len_detection);

#endif /* CHRISTOV_H_ */
