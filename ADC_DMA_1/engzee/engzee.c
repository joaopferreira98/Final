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

///**
// * @brief Diferenciacao de Engzee
// * @input valores apos filtragem
// * @output valores diferenciados - Engzee
// */
float* engzee_differentiation(float *input, int length) {
	float *diff_E = (float *)malloc(length * sizeof(float));
	for (int i = 0; i < 4; i++){
		diff_E[i] = 0.0;
	}
	// Acredito que os valores não tem o ABS mesmo, confirmar depois
	for (int i = 4; i < length; i++) {
		diff_E[i] = input[i] - input[i - 4];
	}
	return diff_E;
}


/**
 * @brief Detecção dos picos por Engzee
 * @input Sinal digitalizado, sinal filtrado e diferenciado, tamanho do sinal
 * @output Picos detectados
 */
// Diff_E -> depois alterar para low_pass
int* engzee_lourenco(uint16_t* mock_input, float* diff_E, int length, int fs, int *len_detection) {
	int ms50 = 12;    // 12.5
	int ms200 = 50;     // 50
	int ms350 = 87;    // 87.5
	int ms1200 = 300;    // 300
	int ms160 = 40;
	int neg_threshold = 2;

	float M_slope[250];
	float M = 0;
	float MM[5] = {0}; // initialized to 0
	float M_list[length];
	float neg_m[length];
	int QRS[length]; //Não precisa ser desse tamanho
	int qrs_index = 0;
	int thi_list[length];
	int thi = 0;
	int counter = 0;
	int thf_list[length];
	int thf = 0;
	int thf_index = 0;
	float newM5 = 0;
	int unfiltered_section[25] = {0};
	int section_index = 0;
	int peaks_index = 0;
	int maxi;
	int *r_peaks = (int)malloc(length * sizeof(int));

	float increment = 0.0016064257028112205;

	for (int j = 0; j < ms1200 - ms200; ++j) {
		M_slope[j] = 1.0 - j * increment;
	}
	// Loop para detecção especifícamente do ENGZEE
	for (int i = 0; i < length; i++) {
		//------------------------- AQUI EH PARA ENCONTRAR M -----------------------------
		//-------------------------- CONDIÇÃO 1250 AMOSTRAS ------------------------------
		if (i < 5 * fs) {
			M = 0.6 * max(diff_E, i + 1);
			if (i < 5){
				MM[i] = M;
			}else if (i >= 5){ // i >= 5 ? Testar se isso tá certo
				for (int j = 0; j < 4; j++) {
					MM[j] = MM[j + 1];
				}
				MM[4] = M;
			}

		}
		//------------------------ OUTRAS CONDIÇÕES AQUI ---------------------------------
		//------------------------------ELIF 1-----------------------------------------------
		else if (qrs_index && i < QRS[qrs_index - 1] + ms200) {
			newM5 = 0.6 * max2(diff_E, QRS[qrs_index - 1], i);
			if (newM5 > 1.5 * MM[4]) {
				newM5 = 1.1 * MM[4];
			}
		}
		//------------------------------ELIF 2-----------------------------------------------
		else if (newM5 != 0 && qrs_index && i == QRS[qrs_index - 1] + ms200) {
			if (i >= 5){ // i >= 5 ? Testar se vai dar bom
				for (int j = 0; j < 4; j++) {
					MM[j] = MM[j + 1];
				}
				MM[4] = newM5;
			}
			M = (MM[0] + MM[1] + MM[2] + MM[3] + MM[4]) / 5;
		}
		//---------------------------ELIF 3-----------------------------------------------------
		else if (qrs_index && i > QRS[qrs_index - 1] + ms200 && i < QRS[qrs_index - 1] + ms1200) {
			M = ((MM[0] + MM[1] + MM[2] + MM[3] + MM[4]) / 5) * M_slope[i - (QRS[qrs_index - 1] + ms200)];
		}
		//------------------------------ELIF 4-----------------------------------------------
		else if (qrs_index && i > QRS[qrs_index - 1] + ms1200) {
			M = 0.6 * ((MM[0] + MM[1] + MM[2] + MM[3] + MM[4]) / 5);
		}
		//------------------------ OUTRAS CONDIÇÕES AQUI ---------------------------------
		M_list[i] = M;
		neg_m[i] = -M;
		//------------------------------ DETECÇÃO ----------------------------------------
		if (!qrs_index && diff_E[i] > M) {
			QRS[qrs_index] = i;
			thi_list[qrs_index] = i;
			thi = 1;
			qrs_index++;
		}
		else if (qrs_index && i > QRS[qrs_index - 1] + ms200 && diff_E[i] > M) {
			QRS[qrs_index] = i;
			thi_list[qrs_index] = i;
			thi = 1;
			qrs_index++;
		}
		//------------------------------- THI e THF -------------------------------------
		if (thi && i < thi_list[qrs_index - 1] + ms160) {
			if (diff_E[i] < -M && diff_E[i - 1] > -M) {
				thf = 1;
			}
			if (thf && diff_E[i] < -M) {
				thf_list[thf_index] = i;
				counter++;
				thf_index++;
			}
			else if (diff_E[i] > -M && thf) {
				counter = 0;
				thi = 0;
				thf = 0;
			}
		}
		else if (thi && i > thi_list[qrs_index - 1] + ms160) {
			counter = 0;
			thi = 0;
			thf = 0;
		}
		//-------------------------- ENCONTRAR OS PICOS DE FATO ---------------------------
		if (counter > neg_threshold) {
			for (int k = thi_list[qrs_index -1] - 2; k < i; k++) { // O -2 é pq é -int(0.01*fs)
				unfiltered_section[section_index] = mock_input[k];
				section_index++;
			}
			// Criar função para encontrar o indice do maior valor
			maxi = indexMax(unfiltered_section, 25);

			r_peaks[peaks_index++] = maxi + thi_list[qrs_index -1] - neg_threshold;
			counter = 0;
			thi = 0;
			thf = 0;
			section_index = 0;
			memset(unfiltered_section, 0, 25 * sizeof(int));
		}

	}
	// Remover o primeiro elemento do array r_peaks e realocar a memória
	if (peaks_index > 0) {
		for (int i = 0; i < peaks_index - 1; i++) {
			r_peaks[i] = r_peaks[i + 1];
		}
		peaks_index--;
		int *temp = realloc(r_peaks, peaks_index * sizeof(int));
		if (temp == NULL && peaks_index > 0) {
			perror("Failed to reallocate memory");
			exit(EXIT_FAILURE);
		}
		r_peaks = temp;
	}
	*len_detection += peaks_index;
	return r_peaks;
}


