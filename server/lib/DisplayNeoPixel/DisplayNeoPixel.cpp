
#include "DisplayNeoPixel.h"

#ifdef HAS_NEOPIXELS

// This is optional hardware, four neopixels on pin 26, using 3.3V<->5V level shifter:
// It is a luxury, just more visible status indication. LCD alone is sufficient.

#include <Adafruit_NeoPixel.h>

// ================= Neopixel strip of 4 LEDs ==================================
#define NEOPIXEL_PIN 26
#define NPIX 4
// intensity 1..255 normal and max:
#define INTENSITY 5
#define INTENSITY_MAX 50

// Neopixel assignments, left to right:
#define PIX_RTCMS 3
#define PIX_WIFI 2
#define PIX_CASTER 1
#define PIX_FLOW 0

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NPIX, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setupNeopixels()
{
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void stripAllPixelsOff()
{
  // get them glow dim white:
  colorWipe(strip.Color(1, 1, 1), 1);
}

uint32_t pixColorRed()
{
  return strip.Color(INTENSITY * 2, 0, 0);
}

uint32_t pixColorGreen()
{
  return strip.Color(0, INTENSITY, 0);
}

uint32_t pixColorBlue()
{
  return strip.Color(0, 0, INTENSITY);
}

uint32_t pixColorBlueMax()
{
  return strip.Color(0, 0, INTENSITY_MAX);
}

uint32_t pixColorYellow()
{
  return strip.Color(INTENSITY * 2, INTENSITY, 0);
}

void stripPixelOn(int pixNo, uint32_t color)
{
  strip.setPixelColor(pixNo, color);
  strip.show();
  delay(1);
}

void testNeopixels()
{
#define INTENSITY_TEST 50
#define DURATION 500

  colorWipe(strip.Color(INTENSITY_TEST, 0, 0), DURATION); // Red
  colorWipe(strip.Color(0, INTENSITY_TEST, 0), DURATION); // Green
  colorWipe(strip.Color(0, 0, INTENSITY_TEST), DURATION); // Blue
  stripAllPixelsOff();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
#endif // HAS_NEOPIXELS
