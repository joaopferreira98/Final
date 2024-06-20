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

#define MAX_LEN 320
#define MAX_SEARCH_INTERVAL 200

/**
 * @brief Tradeoff
 * @input Christov and Engzee detections and sizes
 * @output final detections after tradeoff
 */
void tradeoff(int* engzee_detection, int len_engzee, int* christov_detection, int len_christov, int* detections, int* len_detections) {
    static int search_interval[MAX_SEARCH_INTERVAL];
    static int missed_beat[MAX_LEN];

    *len_detections = len_engzee;
    for (int i = 0; i < len_engzee; i++) {
        detections[i] = engzee_detection[i];
    }

    if (*len_detections > 20) {
        for (int i = 0; i < *len_detections - 1; i++) {
            int a = detections[i] + 10;
            int b = detections[i + 1] - 10;
            if (b - a < 0) {
                continue;
            }
            int search_interval_len = b - a + 1;
            for (int j = 0; j < search_interval_len; j++) {
                search_interval[j] = a + j;
            }

            int missed_beat_len = 0;
            intersect(search_interval, search_interval_len, christov_detection, len_christov, &missed_beat_len);

            if (missed_beat_len > 0) {
                if (*len_detections + missed_beat_len > MAX_LEN) {
                    exit(1);
                }
                for (int k = 0; k < missed_beat_len; k++) {
                    detections[(*len_detections)++] = missed_beat[k];
                }
            }
        }
    } else {
        for (int i = 0; i < len_christov; i++) {
            detections[i] = christov_detection[i];
        }
        *len_detections = len_christov;
    }

    qsort(detections, *len_detections, sizeof(int), compareInts);
}
