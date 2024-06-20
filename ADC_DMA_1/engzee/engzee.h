/*
 * engzee.h
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#ifndef ENGZEE_H_
#define ENGZEE_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mathematics.h"
#include "prefiltering.h"
#include "christov.h"

void engzee_differentiation(float *input, float *diff_E, int length);
void engzee_lourenco(uint16_t* mock_input, float* diff_E, int length, int sample, int fs, int *r_peaks, int *peaks_index,  float *MM, int *thi_list);

#endif /* ENGZEE_H_ */
