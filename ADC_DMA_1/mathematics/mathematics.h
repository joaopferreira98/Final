/*
 * mathematics.h
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#ifndef MATHEMATICS_H_
#define MATHEMATICS_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


float max(float* arr, int size);
float max2(float* arr, int start, int end);
int indexMax(int* arr, int size);
int compare_ints(const void* a, const void* b);
int* intersect(int* arr1, int len1, int* arr2, int len2, int* result_len);

#endif /* MATHEMATICS_H_ */
