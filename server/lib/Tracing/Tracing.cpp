#include "Tracing.h"

void Trace(String str)
{
  printPrefix();
  Serial.println(str);
}

void Error(String str)
{
  printPrefix();
  Serial.print("Error: ");
  Serial.println(str);
}
void Info(String str)
{
  printPrefix();
  Serial.print("Info: ");
  Serial.println(str);
}

void printPrefix()
{
  Serial.print("s: ");
  Serial.print(((double)millis()) / 1000.0, 3);
  Serial.print(" | ");
}
