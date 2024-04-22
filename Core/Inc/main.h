/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_agent.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#ifndef TRUE
	#define TRUE (1)
#endif
#ifndef FALSE
	#define FALSE (0)
#endif

//#define DEBUG_MODE

#ifdef DEBUG_MODE
#include <string.h>
//#define TRACELOG(LOG) USBD_VPC_Send((uint8_t*)(LOG),strlen((char*)(LOG)))
#define TRACELOG(LOG)  VPC_Transmit((LOG),strlen((char*)(LOG)))

#else
#define TRACELOG(LOG) {}	
#endif

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED0_0_Pin GPIO_PIN_4
#define LED0_0_GPIO_Port GPIOA
#define LED30_Pin GPIO_PIN_5
#define LED30_GPIO_Port GPIOA
#define LED50_Pin GPIO_PIN_6
#define LED50_GPIO_Port GPIOA
#define LED03_Pin GPIO_PIN_7
#define LED03_GPIO_Port GPIOA
#define LED_00_Pin GPIO_PIN_0
#define LED_00_GPIO_Port GPIOB
#define LED01_Pin GPIO_PIN_1
#define LED01_GPIO_Port GPIOB
#define LED60_Pin GPIO_PIN_10
#define LED60_GPIO_Port GPIOB
#define LED70_Pin GPIO_PIN_11
#define LED70_GPIO_Port GPIOB
#define IRQ_Pin GPIO_PIN_12
#define IRQ_GPIO_Port GPIOB
#define IRQ_EXTI_IRQn EXTI15_10_IRQn
#define CSN_Pin GPIO_PIN_8
#define CSN_GPIO_Port GPIOA
#define CE_Pin GPIO_PIN_9
#define CE_GPIO_Port GPIOA
#define LED04_Pin GPIO_PIN_15
#define LED04_GPIO_Port GPIOA
#define LED06_Pin GPIO_PIN_3
#define LED06_GPIO_Port GPIOB
#define LED10_Pin GPIO_PIN_4
#define LED10_GPIO_Port GPIOB
#define LED20_Pin GPIO_PIN_5
#define LED20_GPIO_Port GPIOB
#define LED07_Pin GPIO_PIN_6
#define LED07_GPIO_Port GPIOB
#define LED40_Pin GPIO_PIN_7
#define LED40_GPIO_Port GPIOB
#define LED05_Pin GPIO_PIN_8
#define LED05_GPIO_Port GPIOB
#define LED02_Pin GPIO_PIN_9
#define LED02_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
