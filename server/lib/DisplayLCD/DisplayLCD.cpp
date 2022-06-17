
// LCD is connected in parallel 4-bit way, see setup section below.
// DO NOT share I2C (Qwiic) bus with any other device. Specifically, OLED proved not to work.
#include "DisplayLCD.h"

#include <LiquidCrystal.h>

// ================ DFRobot LCD Display 16x2 Chars ============================================
//
// see https://www.circuitschools.com/interfacing-16x2-lcd-module-with-esp32-with-and-without-i2c/
//     https://diyi0t.com/lcd-display-tutorial-for-arduino-and-esp8266/
//
// for DFRobot "LCD Keypad Shield" or similar, just bare LCD module:
// const int rs = 25, en = 26, d4 = 34, d5 = 35, d6 = 32, d7 = 33; - does not work (XTAL pins??)
const int rs = 19, en = 18, d4 = 4, d5 = 16, d6 = 17, d7 = 5;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setupLcd()
{
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("NTRIP Alabama");
}

// Text on LCD display:

const char *spaces = "                ";

void textLcdLine1(String str)
{
  // set the cursor to column 0, line 0
  // (note: line counting begins with 0):
  lcd.setCursor(0, 0);
  lcd.print(spaces);
  lcd.setCursor(0, 0);
  lcd.print(str);
}

void textLcdLine2(String str)
{
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  lcd.print(spaces);
  lcd.setCursor(0, 1);
  lcd.print(str);
}
