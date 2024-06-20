/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "prefiltering.h"
#include "christov.h"
#include "engzee.h"
#include "tradeoff.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUF_LEN 2500
#define BUF_LEN_HALF 1250
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
uint16_t buffer[BUF_LEN];

uint8_t fill = 0; // 0: buffer vazio; 1: primeira metade cheia; 2: segunda metade cheia

// flags de quantidade de leitura
uint8_t sample1 = 0;
uint8_t sample2 = 1;

// frequency sample
uint8_t fs = 250;

// Variaveis do prefiltering
float filtered_ecg[BUF_LEN_HALF];
uint16_t total_taps = 0;

float diff_C[BUF_LEN_HALF - 2];
float diff_E[BUF_LEN_HALF];
float diff_filtered_C[BUF_LEN_HALF - 2];
float diff_filtered_E[BUF_LEN_HALF];

int engzee_detection[320];
int christov_detection[320];
int len_engzee = 0;
int len_christov = 0;
int detections[320];
int len_detections = 0;

float MM_engzee[5] = {0};
float MM_christov[5] = {0};
float RR[5] = {0};
int R_idx = 0;
int thi_list[320];


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
	MX_TIM2_Init();
	/* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start(&htim2);

	// Atraso de 3 segundos antes de iniciar a aquisição de dados (para teste)
	HAL_Delay(3000);
	//while(HAL_GPIO_ReadPin(PB_GPIO_Port, PB_Pin)); // Quando utilizar o botão para inciar a coleta

	HAL_ADC_Start_DMA(&hadc1, buffer, BUF_LEN);
	while(fill == 0);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/**
		 * @brief Call pre-filtering
		 * @input Digital input, size of half buffer
		 * @output Filtered input
		 */
		prefiltering(&buffer[0], BUF_LEN_HALF, &total_taps, filtered_ecg);
		/**
		 * @brief Christov Differentiation
		 * @input input prefiltered, size of half buffer
		 * @output signal differentiated - Christov
		 */
		christov_differentiation(filtered_ecg, diff_C, BUF_LEN_HALF);
		/**
		 * @brief Engzee Differentiation
		 * @input input prefiltered, size of half buffer
		 * @output signal differentiated - Engzee
		 */
		engzee_differentiation(filtered_ecg, diff_E, BUF_LEN_HALF);
		/**
		 * @brief Call christov_noise to diff C
		 * @input signal differentiated - Christov, size of half buffer, total taps
		 * @output Christov filtered signal
		 */
		chistov_noise(diff_C, diff_filtered_C, total_taps, BUF_LEN_HALF - 2);
		/**
		 * @brief Call christov_noise to diff E
		 * @input signal differentiated - Engzee, size of half buffer, total taps
		 * @output Engzee filtered signal
		 */
		chistov_noise(diff_E, diff_filtered_E, total_taps, BUF_LEN_HALF);
		/**
		 * @brief Call engzee_lourenco to find engzee detections
		 * @input Engzee filtered signal, digital input, len half buffer, relative sample, frequency sample,
		 * parameters from past detection
		 * @output engzee detections
		 */
		engzee_lourenco(&buffer[0], diff_filtered_E, BUF_LEN_HALF, sample1, fs, engzee_detection, &len_engzee, MM_engzee, thi_list);
		/**
		 * @brief Call christov to find christov detections
		 * @input Christov filtered signal, digital input, len half buffer, relative sample, frequency sample,
		 * parameters from past detection
		 * @output christov detections
		 */
		christov(&buffer[0], diff_filtered_C, BUF_LEN_HALF - 2, sample1, fs, christov_detection, &len_christov, MM_christov, RR, &R_idx);

		total_taps = 0;
		sample1 += 2;
		while(fill == 1);

		/**
		 * @brief Call pre-filtering
		 * @input Digital input, size of half buffer
		 * @output Filtered input
		 */
		prefiltering(&buffer[BUF_LEN_HALF], BUF_LEN_HALF, &total_taps, filtered_ecg);
		/**
		 * @brief Christov Differentiation
		 * @input input prefiltered, size of half buffer
		 * @output signal differentiated - Christov
		 */
		christov_differentiation(filtered_ecg, diff_C, BUF_LEN_HALF);
		/**
		 * @brief Engzee Differentiation
		 * @input input prefiltered, size of half buffer
		 * @output signal differentiated - Engzee
		 */
		engzee_differentiation(filtered_ecg, diff_E, BUF_LEN_HALF);
		/**
		 * @brief Call christov_noise to diff C
		 * @input signal differentiated - Christov, size of half buffer, total taps
		 * @output Christov filtered signal
		 */
		chistov_noise(diff_C, diff_filtered_C, total_taps, BUF_LEN_HALF - 2);
		/**
		 * @brief Call christov_noise to diff E
		 * @input signal differentiated - Engzee, size of half buffer, total taps
		 * @output Engzee filtered signal
		 */
		chistov_noise(diff_E, diff_filtered_E, total_taps, BUF_LEN_HALF);
		/**
		 * @brief Call engzee_lourenco to find engzee detections
		 * @input Engzee filtered signal, digital input, len half buffer, relative sample, frequency sample,
		 * parameters from past detection
		 * @output engzee detections
		 */
		engzee_lourenco(&buffer[BUF_LEN_HALF], diff_filtered_E, BUF_LEN_HALF, sample2, fs, engzee_detection, &len_engzee, MM_engzee, thi_list);
		/**
		 * @brief Call christov to find christov detections
		 * @input Christov filtered signal, digital input, len half buffer, relative sample, frequency sample,
		 * parameters from past detection
		 * @output christov detections
		 */
		christov(&buffer[BUF_LEN_HALF], diff_filtered_C, BUF_LEN_HALF - 2, sample2, fs, christov_detection, &len_christov, MM_christov, RR, &R_idx);

		total_taps = 0;
		sample2 += 2;
		if (sample2 == 25){
			fill = 3;
		}
		while(fill == 2);

		if(fill == 3){
			/**
			 * @brief Call tradeoff
			 * @input Christov and Engzee detections and sizes
			 * @output Tradeoff detections
			 */
			tradeoff(engzee_detection, len_engzee, christov_detection, len_christov, detections, &len_detections);
			break;
		}
		/* USER CODE END 3 */
	}
	/* USER CODE END WHILE */

	/* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */


/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 12;
	RCC_OscInitStruct.PLL.PLLN = 96;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void)
{

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = {0};

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = ENABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void)
{

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 100-1;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 4000-1;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	{
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */

}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{

	/* DMA controller clock enable */
	__HAL_RCC_DMA2_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA2_Stream0_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin : PB_Pin */
	GPIO_InitStruct.Pin = PB_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(PB_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
	fill = 1;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	fill = 2;
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
