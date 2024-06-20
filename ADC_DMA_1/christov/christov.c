/*
 * christov.c
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mathematics.h"
#include "prefiltering.h"

#define MAX_LENGTH 1000

/**
 * @brief Christov Differentiation
 * @input values ​​after filtering, sample size
 * @output differentiated values ​​- Christov
 */
void christov_differentiation(float *input, float *diff_C, int length) {
	int count = 0;
	for (int i = 1; i < length - 1; i++) {
		diff_C[count++] = fabs(input[i + 1] - input[i - 1]);
	}
}

/**
 * @brief Call lfilter to diff_signals
 * @input differentiated values
 * @output filtered values
 */
void chistov_noise(float *diff_signal, float *diff_filtered_signal, uint16_t total_taps, int length) {
	float b[10] = { 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 };
	uint16_t a[] = {1};
	uint16_t len_b = sizeof(b) / sizeof(b[0]);
	uint16_t len_a = sizeof(a) / sizeof(a[0]);
	total_taps += len_b;

	floatfilter(b, a, len_b, len_a, diff_signal, diff_filtered_signal, length);

	for (int i = 0; i < total_taps; i++) {
		diff_filtered_signal[i] = 0;
	}
}


/**
 * @brief Christov Detection
 * @input Digitized input, Christov differentiated array, sample buffer index, MM and RR
 * @output Spikes detected - Christov
 */
void christov(uint16_t* mock_input, float* MA3, int length, int sample, int fs, int* QRS, int *len_detection, float *MM, float *RR, int *R_idx) {
	int qrs_index = *len_detection;
	int max_qrs_size = 320;
	float M = 0;
	float newM5 = 0;
	float M_slope[250];
	float F = 0;
	int R = 0;
	int Rm = 0;
	int ms50 = 12;
	int ms200 = 50;
	int ms350 = 87;
	int ms1200 = 300;
	int first = *len_detection;
	int start = (length * sample);
	int idx = *R_idx;
	float increment = 0.0016064257028112205;

	for (int j = 0; j < ms1200 - ms200; ++j) {
		M_slope[j] = 1.0 - j * increment;
	}

	float* F_section = (float*)malloc(ms350 * sizeof(float));

	for (int i = start; i < length * (sample + 1); i++) {
		if ((i-start) < 5 * fs) {
			M = 0.6 * max(MA3, i - start + 1);
			if (i < 5){
				MM[i] = M;
			}else if (i >= 5){
				for (int j = 0; j < 4; j++) {
					MM[j] = MM[j + 1];
				}
				MM[4] = M;
			}
		}
		else if (qrs_index && i < QRS[qrs_index - 1] + ms200) {
			if (QRS[qrs_index - 1] < start){
				newM5 = 0.6 * max2(MA3, 0, (i - start));
			}
			else{
				newM5 = 0.6 * max2(MA3, QRS[qrs_index - 1], (i - start));
			}
			if (newM5 > 1.5 * MM[4]) {
				newM5 = 1.1 * MM[4];
			}
		}
		else if (qrs_index && i == QRS[qrs_index - 1] + ms200) {
			if (newM5 == 0) {
				newM5 = MM[4];
			}
			for (int j = 0; j < 4; j++) {
				MM[j] = MM[j + 1];
			}
			MM[4] = newM5;
			M = (MM[0] + MM[1] + MM[2] + MM[3] + MM[4]) / 5;
		}
		else if (qrs_index && i > QRS[qrs_index - 1] + ms200 && i < QRS[qrs_index - 1] + ms1200) {
			M = ((MM[0] + MM[1] + MM[2] + MM[3] + MM[4]) / 5) * M_slope[i - (QRS[qrs_index - 1] + ms200)];
		}
		else if (qrs_index && i > QRS[qrs_index - 1] + ms1200) {
			M = 0.6 * ((MM[0] + MM[1] + MM[2] + MM[3] + MM[4]) / 5);
		}

		if ((i - start) > ms350) {
			if (F_section == NULL) {
				exit(1);
			}
			for (int j = 0; j < ms350; j++) {
				F_section[j] = MA3[i - start - ms350 + j];
			}
			float max_latest = F_section[ms350 - ms50];
			for (int j = ms350 - ms50 + 1; j < ms350; j++) {
				if (F_section[j] > max_latest) {
					max_latest = F_section[j];
				}
			}
			float max_earliest = F_section[0];
			for (int j = 1; j < ms50; j++) {
				if (F_section[j] > max_earliest) {
					max_earliest = F_section[j];
				}
			}

			F = F + ((max_latest - max_earliest) / 150.0);

		}

		if (qrs_index && i < QRS[qrs_index - 1] + (2.0 / 3.0 * Rm)) {
			R = 0;
		} else if (qrs_index && i > QRS[qrs_index - 1] + (2.0 / 3.0 * Rm) && i < QRS[qrs_index - 1] + Rm) {
			int dec = (M - ((MM[0]+MM[1]+MM[2]+MM[3]+MM[4])/5)) / 1.4;
			R = 0 + dec;
		}

		float MFR = M + F + R;

		if (!qrs_index && MA3[i - start] > MFR) {
			QRS[qrs_index++] = i;
		} else if (qrs_index && i > QRS[qrs_index - 1] + ms200 && MA3[i - start] > MFR) {
			QRS[qrs_index++] = i;

			if (qrs_index > 2) {
				if (idx < 5){
					RR[idx] = (QRS[qrs_index - 1] - QRS[qrs_index - 2]);
					idx++;
				}else{
					for (int j = 0; j < 4; j++) {
						RR[j] = RR[j + 1];
					}
					RR[4] = (QRS[qrs_index - 1] - QRS[qrs_index - 2]);
				}
				Rm = ((RR[0] + RR[1] + RR[2] + RR[3] + RR[4]) / 5);
			}

		}
	}

	free(F_section);

	for (int l = first; l < qrs_index; l++) {
		QRS[l] = QRS[l + 1];
	}
	qrs_index--;

	*len_detection = qrs_index;
	*R_idx = idx;
}
