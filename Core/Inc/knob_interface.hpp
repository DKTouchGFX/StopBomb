#ifndef KNOB_INTERFACE_H
#define KNOB_INTERFACE_H

#include <stdint.h>

void knobUartSendByte(uint8_t data);
void knobUartSendBytes(uint8_t* data, uint16_t size);
void knobUartSendString(uint8_t* data);
uint8_t knobUartReceiveByte(uint8_t* data);
void knobSetAmbientLightRGB(uint8_t red, uint8_t green, uint8_t blue);

#endif //KNOB_INTERFACE_H
