/*
 * prefiltering.c
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#include <stdio.h>
#include <stdlib.h>
#include "filter.h"

/**
 * @brief prefilter
 * @input digital input, size of digital input
 * @output digital input filtered, total taps
 */
void prefiltering(uint16_t *digitalized_ecg, uint16_t length, uint16_t *total_taps, float *filtered_ecg) {
    float b1[] = { 0.2, 0.2, 0.2, 0.2, 0.2 };
    float b2[] = { 0.14285, 0.14285, 0.14285, 0.14285, 0.14285, 0.14285, 0.14285 };
    uint16_t a[] = {1};
    uint16_t len_b1 = sizeof(b1) / sizeof(b1[0]);
    uint16_t len_b2 = sizeof(b2) / sizeof(b2[0]);
    uint16_t len_a = sizeof(a) / sizeof(a[0]);

    *total_taps += len_b1 + len_b2;

    float MA1[length];

    intfilter(b1, a, len_b1, len_a, digitalized_ecg, MA1, length);

    floatfilter(b2, a, len_b2, len_a, MA1, filtered_ecg, length);
}


