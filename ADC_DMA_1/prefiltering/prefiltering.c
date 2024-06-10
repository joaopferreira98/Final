/*
 * prefiltering.c
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#include <stdio.h>
#include <stdlib.h>
#include "filter.h"

#define FS 250

float* prefiltering(uint16_t *digitalized_ecg, uint16_t length, uint16_t *total_taps) {
    // Define filter coefficients and lengths
    float b1[] = { 0.2, 0.2, 0.2, 0.2, 0.2 };  // Power line interference filter coefficients
    float b2[] = { 0.14285, 0.14285, 0.14285, 0.14285, 0.14285, 0.14285, 0.14285 };  // EMG noise filter coefficients
    uint16_t a[] = {1};  // Denominator coefficients (always 1 for this application)
    uint16_t len_b1 = sizeof(b1) / sizeof(b1[0]);
    uint16_t len_b2 = sizeof(b2) / sizeof(b2[0]);
    uint16_t len_a = sizeof(a) / sizeof(a[0]);
//    uint16_t fs = 250;

    // Calculate total taps for both filters
    *total_taps += len_b1 + len_b2;

    // Filter power line interference
    float *MA1 = (float *)malloc(length * sizeof(float));
    intfilter(b1, a, len_b1, len_a, digitalized_ecg, MA1, length);

    // Filter EMG noise
    float *filtered_ecg = (float *)malloc(length * sizeof(float));
    floatfilter(b2, a, len_b2, len_a, MA1, filtered_ecg, length);

    // Free memory
    free(MA1);

    return filtered_ecg;
}

