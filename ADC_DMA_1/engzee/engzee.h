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

float* engzee_differentiation(const float *input, int length);
int* engzee_lourenco(uint16_t* mock_input, float* diff_E, int length, int fs, int *len_detection);

#endif /* ENGZEE_H_ */
