/*
 * tradeoff.c
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


int* tradeoff(int* engzee_detection, int len_engzee, int* christov_detection, int len_christov) {
	int* detections = engzee_detection;
	int len_detections = len_engzee;

	if (len_detections > 20) {
		for (int i = 0; i < len_detections; i++) {
			int a = detections[i] + 10;
			int b = detections[i + 1] - 10;
			if (b - a < 0) {
				continue;
			}
			int search_interval_len = b - a + 1;
			int* search_interval = (int*)malloc(search_interval_len * sizeof(int));
			for (int j = 0; j < search_interval_len; j++) {
				search_interval[j] = a + j;
			}

			int missed_beat_len = 0;
			int* missed_beat = intersect(search_interval, search_interval_len, christov_detection, len_christov, &missed_beat_len);

			if (missed_beat_len > 0) {
				detections = (int*)realloc(detections, (len_detections + missed_beat_len) * sizeof(int));
				for (int k = 0; k < missed_beat_len; k++) {
					detections[len_detections++] = missed_beat[k];
				}
			}
			free(search_interval);
			free(missed_beat);
		}
	} else {
		detections = christov_detection;
		len_detections = len_christov;
	}

	qsort(detections, len_detections, sizeof(int), compare_ints);
	return detections;
}
