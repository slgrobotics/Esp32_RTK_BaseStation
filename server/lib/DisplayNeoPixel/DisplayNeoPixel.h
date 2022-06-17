#ifndef ELYIR_DISPLAY_NEOPIXEL_H
#define ELYIR_DISPLAY_NEOPIXEL_H

// This is optional hardware, four neopixels on pin 26, using 3.3V<->5V level shifter:
// It is a luxury, just more visible status indication. LCD alone is sufficient.

#include <Adafruit_NeoPixel.h>

void setupNeopixels();

void stripAllPixelsOff();

uint32_t pixColorRed();

uint32_t pixColorGreen();
uint32_t pixColorBlue();

uint32_t pixColorBlueMax();

uint32_t pixColorYellow();
void stripPixelOn(int pixNo, uint32_t color);
void testNeopixels();

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait);

#endif
