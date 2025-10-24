#include "main.h"
#include "knob_interface.hpp"
#include <stm32c0xx_ll_usart.h>
#include "stm32c0xx_ll_tim.h"

static USART_TypeDef* const UART_COM = USART1;

void knobUartSendByte(uint8_t data)
{
    while (!LL_USART_IsActiveFlag_TXE_TXFNF(UART_COM)) // Wait until there is room in the TX FIFO (it can hold up to 8 bytes)
    {}
    LL_USART_TransmitData8(UART_COM, data);
}

void knobUartSendBytes(uint8_t* data, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        knobUartSendByte(data[i]);
    }
}

void knobUartSendString(uint8_t* data)
{
    uint16_t i = 0;
    while (data[i] != 0)
    {
        knobUartSendByte(data[i]);
        i++;
    }
}

uint8_t knobUartReceiveByte(uint8_t* data)
{
    if (LL_USART_IsActiveFlag_RXNE_RXFNE(UART_COM))
    {
        *data = LL_USART_ReceiveData8(UART_COM);
        return 1;
    }
    else
    {
        return 0;
    }
}

void knobSetAmbientLightRGB(uint8_t red, uint8_t green, uint8_t blue)
{
    static uint8_t isInitializedPWM = 0;

    // Start the timers if not already started
    if (!isInitializedPWM)
    {
        LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1N);
        LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2N);
        LL_TIM_EnableAllOutputs(TIM1);
        LL_TIM_EnableCounter(TIM1);

        LL_TIM_CC_EnableChannel(TIM17, LL_TIM_CHANNEL_CH1);
        LL_TIM_EnableAllOutputs(TIM17);
        LL_TIM_EnableCounter(TIM17);

        isInitializedPWM = 1;
    }
    // Set the RGB values for the ambient light using PWM
    LL_TIM_OC_SetCompareCH1(TIM1, green);
    LL_TIM_OC_SetCompareCH2(TIM1, blue);
    LL_TIM_OC_SetCompareCH1(TIM17, red);
}
