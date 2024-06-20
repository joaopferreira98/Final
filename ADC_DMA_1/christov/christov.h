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

void christov_differentiation(float *input, float *diff_C, int length);
void chistov_noise(float *diff_signal, float *diff_filtered_signal, uint16_t total_taps, int length);
void christov(uint16_t* mock_input, float* MA3, int length, int sample, int fs, int* QRS, int *len_detection, float *MM, float *RR, int *R_idx);

#endif /* CHRISTOV_H_ */
