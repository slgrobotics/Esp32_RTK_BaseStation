#include <Arduino.h>
#include "Controls.h"

// Controls: Idle button/switch, connected to D13:

const int idleSwitchPin = 13; // Green pushbutton pin, paired with a switch above it (switch up=button pressed)
                              // Has a pull-up resistor 1-10K, button or switch grounds the pin.
                              // Red pushbutton = Reset (a.k.a. EN)
// const int OTHR_ButtonPin = 14;

void setupControls()
{
  // initialize the pushbutton pin as an input:
  pinMode(idleSwitchPin, INPUT);
}

bool isIdleSwitchOn()
{
  int buttonState = digitalRead(idleSwitchPin);

  return buttonState == 0; // pin grounded
}
