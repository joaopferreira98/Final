/*
 * engzee.c
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
#include "christov.h"

/**
 * @brief Engzee Differentiation
 * @input values ​​after filtering
 * @output differentiated values ​​- Engzee
 */
void engzee_differentiation(float *input, float *diff_E, int length) {
	for (int i = 0; i < 4; i++){
		diff_E[i] = 0.0;
	}
	for (int i = 4; i < length; i++) {
		diff_E[i] = input[i] - input[i - 4];
	}
}

/**
 * @brief Engzee Detection
 * @input Digitized input, Engzee differentiated array, sample buffer index, MM and Thi_list
 * @output Spikes detected - Engzee
 */
void engzee_lourenco(uint16_t* mock_input, float* diff_E, int length, int sample, int fs, int *r_peaks, int *peaks_index, float *MM, int *thi_list) {
	int ms200 = 50;
	int ms1200 = 300;
	int ms160 = 40;
	int neg_threshold = 2;
	int max_qrs_size = 320;
	int max_section_size = 25;
	float M_slope[250];
	float M = 0;
	int QRS[max_qrs_size];
	int qrs_index = 0;
	int thi = 0;
	int counter = 0;
	int thf = 0;
	float newM5 = 0;
	int unfiltered_section[25] = {0};
	int section_index = 0;
	int maxi;

	int first = *peaks_index;
	int start = (length * sample);

	float increment = 0.0016064257028112205;

	for (int j = 0; j < ms1200 - ms200; ++j) {
		M_slope[j] = 1.0 - j * increment;
	}

	for (int i = start; i < (length * (sample + 1)); i++) {
		//------------------------- AQUI EH PARA ENCONTRAR M -----------------------------
		if (i < 5 * fs) {
			M = 0.6 * max(diff_E, i + 1);
			if (i < 5){
				MM[i] = M;
			} else if (i >= 5) {
				for (int j = 0; j < 4; j++) {
					MM[j] = MM[j + 1];
				}
				MM[4] = M;
			}
		}
		//------------------------------ELIF 1-----------------------------------------------
		else if (qrs_index && i < QRS[qrs_index - 1] + ms200) {
			if (QRS[qrs_index - 1] < start){
				newM5 = 0.6 * max2(diff_E, 0, (i - start));
			}
			else{
				if ((i - start) - QRS[qrs_index - 1]){
					newM5 = 0;
					}
				else{
					newM5 = 0.6 * max2(diff_E, QRS[qrs_index - 1], (i - start));

				}
			}
			if (newM5 > 1.5 * MM[4]) {
				newM5 = 1.1 * MM[4];
			}
		}
		//------------------------------ELIF 2-----------------------------------------------
		else if (newM5 != 0 && qrs_index && i == QRS[qrs_index - 1] + ms200) {
			if (i >= 5){
				for (int j = 0; j < 4; j++) {
					MM[j] = MM[j + 1];
				}
				MM[4] = newM5;
			}
			M = (MM[0] + MM[1] + MM[2] + MM[3] + MM[4]) / 5;
		}
		//------------------------------ELIF 3-------------------------------------------
		else if (qrs_index && i > QRS[qrs_index - 1] + ms200 && i < QRS[qrs_index - 1] + ms1200) {
			M = ((MM[0] + MM[1] + MM[2] + MM[3] + MM[4]) / 5) * M_slope[i - (QRS[qrs_index - 1] + ms200)];
		}
		//------------------------------ELIF 4-------------------------------------------
		else if (qrs_index && i > QRS[qrs_index - 1] + ms1200) {
			M = 0.6 * ((MM[0] + MM[1] + MM[2] + MM[3] + MM[4]) / 5);
		}
		//------------------------------ DETECÇÃO ----------------------------------------
		if (!qrs_index && diff_E[(i-start)] > M) {
			QRS[qrs_index] = i;
			thi_list[qrs_index] = i;
			thi = 1;
			qrs_index++;
		} else if (qrs_index && i > QRS[qrs_index - 1] + ms200 && diff_E[(i-start)] > M) {
			QRS[qrs_index] = i;
			thi_list[qrs_index] = i;
			thi = 1;
			qrs_index++;
		}
		//------------------------------- THI e THF -------------------------------------
		if (thi && i < thi_list[qrs_index - 1] + ms160) {
			if (diff_E[(i-start)] < -M && diff_E[i - start - 1] > -M) {
				thf = 1;
			}
			if (thf && diff_E[(i-start)] < -M) {
				counter++;
			} else if (diff_E[(i-start)] > -M && thf) {
				counter = 0;
				thi = 0;
				thf = 0;
			}
		} else if (thi && i > thi_list[qrs_index - 1] + ms160) {
			counter = 0;
			thi = 0;
			thf = 0;
		}
		//-------------------------- ENCONTRAR OS PICOS DE FATO ---------------------------
		if (counter > neg_threshold) {
			for (int k = thi_list[qrs_index - 1] - 2; k < i; k++) {
				unfiltered_section[section_index] = mock_input[k];
				section_index++;
			}
			maxi = indexMax(unfiltered_section, max_section_size);

			r_peaks[*peaks_index] = maxi + thi_list[qrs_index - 1] - neg_threshold;
			(*peaks_index)++;
			counter = 0;
			thi = 0;
			thf = 0;
			section_index = 0;
			memset(unfiltered_section, 0, max_section_size * sizeof(int));
		}
	}
	if (first == 0){
		for (int l = 0; l < (*peaks_index); l++) {
			r_peaks[l] = r_peaks[l + 1];
		}
		first++;
		(*peaks_index)--;
	}

}
