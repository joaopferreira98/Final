/*
 * prefiltering.h
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#ifndef PREFILTERING_H_
#define PREFILTERING_H_

#include <stdio.h>
#include <stdlib.h>
#include "filter.h"

void prefiltering(uint16_t *digitalized_ecg, uint16_t length, uint16_t *total_taps, float *filtered_ecg);

#endif /* PREFILTERING_H_ */
