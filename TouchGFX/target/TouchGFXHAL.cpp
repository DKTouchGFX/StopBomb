/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : TouchGFXHAL.cpp
  ******************************************************************************
  * This file was created by TouchGFX Generator 4.24.0. This file is only
  * generated once! Delete this file from your project and re-generate code
  * using STM32CubeMX or change this file manually to update it.
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

#include <TouchGFXHAL.hpp>

/* USER CODE BEGIN TouchGFXHAL.cpp */
#include <touchgfx/hal/OSWrappers.hpp>
#include "stm32c0xx.h"
#include <KeySampler.hpp>
#include <touchgfx/hal/GPIO.hpp>
#include "main.h"
#include "RVA15MD_DisplayDriver.h"
#include "stm32c0xx_ll_tim.h"

static TIM_TypeDef* const LINE_TIMER = TIM16;

using namespace touchgfx;

KeySampler btnctrl;
volatile uint16_t tearingEffectCount = 0;
extern volatile uint32_t rotateLeft;
extern volatile uint32_t rotateRight;

void TouchGFXHAL::initialize()
{
    // Calling parent implementation of initialize().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.
    // Please note, HAL::initialize() must be called to initialize the framework.

    /* Initialize display driver */
    DisplayDriver_Init();

    /* Prepares Display for operation */
    DisplayDriver_DisplayReset();
    DisplayDriver_DisplayInit();

    /* Initialize TouchGFX Engine */
    TouchGFXGeneratedHAL::initialize();
    setButtonController(&btnctrl);

    /* Render first frame, so there is valid data in the display's GRAM */
    LL_TIM_SetCounter(LINE_TIMER, 0xFFFF); // allow all display lines to be written
    HAL::getInstance()->backPorchExited();

    /* GRAM has been filled, turn on display to show content of GRAM */
    DisplayDriver_DisplayOn();
    touchgfx::OSWrappers::signalRenderingDone();
}

/**
 * Gets the frame buffer address used by the TFT controller.
 *
 * @return The address of the frame buffer currently being displayed on the TFT.
 */
uint16_t* TouchGFXHAL::getTFTFrameBuffer() const
{
    // Calling parent implementation of getTFTFrameBuffer().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    return TouchGFXGeneratedHAL::getTFTFrameBuffer();
}

/**
 * Sets the frame buffer address used by the TFT controller.
 *
 * @param [in] address New frame buffer address.
 */
void TouchGFXHAL::setTFTFrameBuffer(uint16_t* address)
{
    // Calling parent implementation of setTFTFrameBuffer(uint16_t* address).
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::setTFTFrameBuffer(address);
}

/**
 * This function is called whenever the framework has performed a partial draw.
 *
 * @param rect The area of the screen that has been drawn, expressed in absolute coordinates.
 *
 * @see flushFrameBuffer().
 */
void TouchGFXHAL::flushFrameBuffer(const touchgfx::Rect& rect)
{
    // Calling parent implementation of flushFrameBuffer(const touchgfx::Rect& rect).
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.
    // Please note, HAL::flushFrameBuffer(const touchgfx::Rect& rect) must
    // be called to notify the touchgfx framework that flush has been performed.
    // To calculate the start address of rect,
    // use advanceFrameBufferToRect(uint8_t* fbPtr, const touchgfx::Rect& rect)
    // defined in TouchGFXGeneratedHAL.cpp

    TouchGFXGeneratedHAL::flushFrameBuffer(rect);
}

bool TouchGFXHAL::beginFrame()
{
    tearingEffectCount = 0;
    return TouchGFXGeneratedHAL::beginFrame();
}

void TouchGFXHAL::endFrame()
{
    TouchGFXGeneratedHAL::endFrame();
    if (tearingEffectCount > 0)
    {
        touchgfx::OSWrappers::signalVSync();
    }
}

bool TouchGFXHAL::blockCopy(void* RESTRICT dest, const void* RESTRICT src, uint32_t numBytes)
{
    return TouchGFXGeneratedHAL::blockCopy(dest, src, numBytes);
}

/**
 * Configures the interrupts relevant for TouchGFX. This primarily entails setting
 * the interrupt priorities for the DMA and LCD interrupts.
 */
void TouchGFXHAL::configureInterrupts()
{
    // Calling parent implementation of configureInterrupts().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::configureInterrupts();
}

/**
 * Used for enabling interrupts set in configureInterrupts()
 */
void TouchGFXHAL::enableInterrupts()
{
    // Calling parent implementation of enableInterrupts().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::enableInterrupts();
}

/**
 * Used for disabling interrupts set in configureInterrupts()
 */
void TouchGFXHAL::disableInterrupts()
{
    // Calling parent implementation of disableInterrupts().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::disableInterrupts();
}

/**
 * Configure the LCD controller to fire interrupts at VSYNC. Called automatically
 * once TouchGFX initialization has completed.
 */
void TouchGFXHAL::enableLCDControllerInterrupt()
{
    // Calling parent implementation of enableLCDControllerInterrupt().
    //
    // To overwrite the generated implementation, omit the call to the parent function
    // and implement the needed functionality here.

    TouchGFXGeneratedHAL::enableLCDControllerInterrupt();
}

extern "C"
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == DISPLAY_TEARING_EFFECT_Pin)
    {
        // The draw speed of displays varies based on factors such as design, production variance, temperature and so on
        // to achieve good performance on all displays the timer tick rate needs to be adjusted based on the actual draw time of the latest frame
        // The following code dynamically adjusts the prescaler on every new frame to match the draw rate
        const uint32_t TARGET = 240; // Number of lines in display drawing direction
        volatile uint32_t count = LL_TIM_GetCounter(LINE_TIMER);
        if (count < TARGET - 1 || count > TARGET + 1)
        {
            const uint16_t oldPrescaler = LL_TIM_GetPrescaler(LINE_TIMER);
            const uint16_t newPrescaler = (count * oldPrescaler + TARGET / 2) / TARGET;

            // Check if the proposed new prescaler is in a reasonable range
            if (newPrescaler > 2000 && newPrescaler < 5000)
            {
                LL_TIM_SetPrescaler(LINE_TIMER, newPrescaler);
                LL_TIM_GenerateEvent_UPDATE(LINE_TIMER); // Reinitialize the counter and generates an update of the registers
            }
        }
        LL_TIM_DisableCounter(LINE_TIMER);
        LL_TIM_SetCounter(LINE_TIMER, 0);

        tearingEffectCount++;

        /* VSync has occurred, increment TouchGFX engine vsync counter */
        touchgfx::HAL::getInstance()->vSync();

        /* VSync has occurred, signal TouchGFX engine */
        touchgfx::OSWrappers::signalVSync();

        GPIO::set(GPIO::VSYNC_FREQ);
    }
    else if (GPIO_Pin == KEY_A_Pin)
    {
        if (HAL_GPIO_ReadPin(KEY_B_GPIO_Port, KEY_B_Pin))
        {
            rotateLeft++;
        }
        else
        {
            rotateRight++;
        }
    }
}

extern "C"
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == DISPLAY_TEARING_EFFECT_Pin)
    {
        LL_TIM_EnableCounter(LINE_TIMER);
        GPIO::clear(GPIO::VSYNC_FREQ);
    }
    else if (GPIO_Pin == KEY_A_Pin)
    {
        if (HAL_GPIO_ReadPin(KEY_B_GPIO_Port, KEY_B_Pin))
        {
            rotateRight++;
        }
        else
        {
            rotateLeft++;
        }
    }
}

extern "C"
int touchgfxDisplayDriverShouldTransferBlock(uint16_t bottom)
{
    // Only allow block transfer if the display has drawn past the bottom of the requested block (plus a margin of two lines)
    // A timer is used to estimate how many lines have been drawn by setting the prescaler so the tick rate matches the line draw rate
    uint16_t lastLineDrawn = LL_TIM_GetCounter(LINE_TIMER);
    return bottom + 2 < lastLineDrawn || tearingEffectCount > 0;
}

/* USER CODE END TouchGFXHAL.cpp */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
