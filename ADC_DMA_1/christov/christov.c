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

/**
 * @brief Diferenciacao de Chrisyov
 * @input valores apos filtragem, tamanho da amostra
 * @output valores diferenciados - Christov
 */
float* christov_differentiation(float *input, int length) {
	float *diff_C = (float *)malloc(length * sizeof(float));
	int count = 0;
	for (int i = 1; i < length - 1; i++) {
		diff_C[count++] = fabs(input[i + 1] - input[i - 1]);
	}
	return diff_C;
}

/**
 * @brief Call lfilter to diff_signals
 * @input valores diferenciados
 * @output valores diferenciados e filtrados
 */

float* chistov_noise(float *diff_signal, uint16_t total_taps, int length) {
	float *diff_filtered_signal = (float *)malloc(length * sizeof(float));

	// Define filter coefficients and lengths
	float b[10] = { 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 };  // Filter coefficients
	uint16_t a[] = {1};  // Denominator coefficients (always 1 for this application)
	uint16_t len_b = sizeof(b) / sizeof(b[0]);
	uint16_t len_a = sizeof(a) / sizeof(a[0]);
	total_taps += len_b;
	// Apply Chistov noise filtering
	floatfilter(b, a, len_b, len_a, diff_signal, diff_filtered_signal, length);

	for (int i = 0; i < total_taps;i++){
		diff_filtered_signal[i] = 0;
	}
	return diff_filtered_signal;
}


///**
// * @brief Detecção dos picos por Christov
// * @input Sinal filtrado e diferenciado, tamanho do sinal
// * @output Picos detectados
// */

int* christov(uint16_t* mock_input, float* MA3, int length, int fs, int *len_detection) {
	int qrs_index = 0;
	float M = 0;
	float newM5 = 0;
	float* M_list;
	float* MM;
	float M_slope[250];
	float F = 0;
	float* F_list;
	int R = 0;
	int* R_list;
	int Rm = 0;
	// Variável e listas para armazenar o resultado final
	int MFR = 0;
	int* MFR_list;
	int *QRS = (int)malloc(length * sizeof(int));
	// Variáveis de intervalos dos tempos 50, 200, 350 e 1200 ms
	int ms50 = 12;    // 12.5
	int ms200 = 50;     // 50
	int ms350 = 87;    // 87.5
	int ms1200 = 300;    // 300
	int ms160 = 40;
	int neg_threshold = 2;

	//	int length = MOCK_INPUT_LEN - 2;
	float* F_section = (float*)malloc(ms350 * sizeof(float)); // Alocando memória para F_section

	// Variáveis e listas para uso do algoritmo
	M_list = (int*)malloc(length * sizeof(int));
	F_list = (float*)malloc(length * sizeof(float));
	R_list = (int*)malloc(length * sizeof(int));
	MM = (float*)malloc((ms1200 - ms200) * sizeof(float));

	// Variável e listas para armazenar o resultado final
	MFR_list = (int*)malloc(length * sizeof(int));
	//	QRS = (int*)malloc(length * sizeof(int));

	float increment = 0.0016064257028112205;

	for (int j = 0; j < ms1200 - ms200; ++j) {
		M_slope[j] = 1.0 - j * increment;
	}

	for (int i = 0; i < length; i++) {
		// M (amplitude máxima) na janela de 5 segundos. A lista MM mantém registro dos
		// valores M anteriores para uso futuro.
		if (i < 5 * fs) {
			M = 0.6 * max(MA3, i + 1);
			MM[i] = M;
			if (i < 5){
				MM[i] = M;
			}else if (i >= 5){ // i >= 5 ? Testar se isso tá certo
				for (int j = 0; j < 4; j++) {
					MM[j] = MM[j + 1];
				}
				MM[4] = M;
			}
		}
		else if (qrs_index && i < QRS[qrs_index	- 1] + ms200) {
			newM5 = 0.6 * max2(MA3, QRS[qrs_index - 1], i);
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
		// (F) Este bloco de código calcula o valor F, que representa a mudança de amplitude dentro
		// de uma janela de tempo específica.
		if (i > ms350) {
			if (F_section == NULL) {
				// Lidar com falha de alocação de memória, se necessário
				exit(1);
			}

			// Copiando os elementos relevantes de MA3 para F_section
			// ISSO PARECE OK COM O PYTHON - MAS DEPOIS PODEMOS VER A FUNDO
			for (int j = 0; j < ms350; j++) {
				F_section[j] = MA3[i - ms350 + j];
			}

			// Encontrando o máximo dos últimos ms50 elementos de F_section
			float max_latest = F_section[ms350 - ms50];
			for (int j = ms350 - ms50 + 1; j < ms350; j++) {
				if (F_section[j] > max_latest) {
					max_latest = F_section[j];
				}
			}

			// Encontrando o máximo dos primeiros ms50 elementos de F_section
			float max_earliest = F_section[0];
			for (int j = 1; j < ms50; j++) {
				if (F_section[j] > max_earliest) {
					max_earliest = F_section[j];
				}
			}

			// Calculando F conforme a fórmula fornecida
			F = F + ((max_latest - max_earliest) / 150.0);

		}
		// R Este bloco de código redefine o valor R se o índice de amostra atual estiver dentro
		// de 2/3 do intervalo R anterior.
		if (qrs_index && i < QRS[qrs_index - 1] + (2.0 / 3.0 * Rm)) {
			R = 0;
		} else if (qrs_index && i > QRS[qrs_index - 1] + (2.0 / 3.0 * Rm) && i < QRS[qrs_index - 1] + Rm) {
			int dec = (M - ((MM[0]+MM[1]+MM[2]+MM[3]+MM[4])/5)) / 1.4;
			R = 0 + dec;
		}

		// Este bloco de código calcula o valor MFR e anexa os valores M, F, R e MFR às suas
		// respectivas listas.
		MFR = M + F + R;
		M_list[i] = M;
		F_list[i] = F;
		R_list[i] = R;
		MFR_list[i] = MFR;

		// Este bloco de código detecta complexos QRS com base na condição de que o sinal de
		// entrada exceda o valor MFR calculado.

		if (!qrs_index && MA3[i] > MFR) {
			QRS[qrs_index++] = i;
			//			aux_6[qrs_index - 1] = QRS[qrs_index - 1];
		} else if (qrs_index && i > QRS[qrs_index - 1] + ms200 && MA3[i] > MFR) {
			QRS[qrs_index++] = i;
			//				printf("breakpoint");
			//			aux_6[qrs_index - 1] = QRS[qrs_index - 1];
			if (qrs_index > 2) {
				Rm = (QRS[qrs_index - 1] - QRS[qrs_index - 2]);
			}
			//				printf("breakpoint");
		}
	}

	// Remover o primeiro elemento do array QRS e realocar a memória
	if (qrs_index > 0) {
		for (int i = 0; i < qrs_index - 1; i++) {
			QRS[i] = QRS[i + 1];
		}
		qrs_index--;
		int *temp = realloc(QRS, qrs_index * sizeof(int));
		if (temp == NULL && qrs_index > 0) {
			perror("Failed to reallocate memory");
			exit(EXIT_FAILURE);
		}
		QRS = temp;
	}
	*len_detection += qrs_index;
	return QRS;
}
