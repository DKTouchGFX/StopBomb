#include "KeySampler.hpp"
#include "main.h"

using namespace touchgfx;

volatile uint32_t rotateLeft;
volatile uint32_t rotateRight;

void KeySampler::init()
{
    rotateLeft = 0;
    rotateRight = 0;
}

bool KeySampler::sample(uint8_t& key)
{
    bool result = false;
    static bool pressedPreviously = false;

    if (rotateLeft > 0)
    {
        rotateLeft--;
        key = '6';
        result = true;
    }
    else if (rotateRight > 0)
    {
        rotateRight--;
        key = '4';
        result = true;
    }
    else if ((HAL_GPIO_ReadPin(KEY_CLICK_GPIO_Port, KEY_CLICK_Pin) == GPIO_PIN_RESET))
    {
        if (!pressedPreviously)
        {
            key = '5';
            pressedPreviously = true;
            result = true;
        }
    }
    else
    {
        pressedPreviously = false;
    }
    return result;
}
