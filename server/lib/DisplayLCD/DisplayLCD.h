
#ifndef ELYIR_DISPLAY_LCD_H
#define ELYIR_DISPLAY_LCD_H

// LCD is connected in parallel 4-bit way, see setup section below.
// DO NOT share I2C (Qwiic) bus with any other device. Specifically, OLED proved not to work.

#include <LiquidCrystal.h>

void setupLcd();

void textLcdLine1(String str);
void textLcdLine2(String str);

#endif
