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
#include "stm32c0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DISPLAY_TEARING_EFFECT_Pin GPIO_PIN_13
#define DISPLAY_TEARING_EFFECT_GPIO_Port GPIOC
#define DISPLAY_TEARING_EFFECT_EXTI_IRQn EXTI4_15_IRQn
#define DISPLAY_RESET_Pin GPIO_PIN_14
#define DISPLAY_RESET_GPIO_Port GPIOC
#define KEY_CLICK_Pin GPIO_PIN_1
#define KEY_CLICK_GPIO_Port GPIOF
#define VSYNC_FREQ_Pin GPIO_PIN_3
#define VSYNC_FREQ_GPIO_Port GPIOA
#define RENDER_TIME_Pin GPIO_PIN_4
#define RENDER_TIME_GPIO_Port GPIOA
#define FRAME_RATE_Pin GPIO_PIN_5
#define FRAME_RATE_GPIO_Port GPIOA
#define DISPLAY_DCX_Pin GPIO_PIN_2
#define DISPLAY_DCX_GPIO_Port GPIOB
#define DISPLAY_CSX_Pin GPIO_PIN_12
#define DISPLAY_CSX_GPIO_Port GPIOB
#define KEY_B_Pin GPIO_PIN_13
#define KEY_B_GPIO_Port GPIOB
#define KEY_A_Pin GPIO_PIN_14
#define KEY_A_GPIO_Port GPIOB
#define KEY_A_EXTI_IRQn EXTI4_15_IRQn
#define LCD_BACKLIGHT_Pin GPIO_PIN_7
#define LCD_BACKLIGHT_GPIO_Port GPIOC
#define LED_RED_Pin GPIO_PIN_1
#define LED_RED_GPIO_Port GPIOD
#define LED_GREEN_Pin GPIO_PIN_2
#define LED_GREEN_GPIO_Port GPIOD
#define LED_BLUE_Pin GPIO_PIN_3
#define LED_BLUE_GPIO_Port GPIOD
#define FLASH_CS_Pin GPIO_PIN_6
#define FLASH_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
