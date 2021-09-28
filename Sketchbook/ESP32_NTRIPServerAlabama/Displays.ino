
void setupDisplays()
{
  setupLeds();

  setupLcd();

#ifdef HAS_NEOPIXELS
  setupNeopixels();
  // spend a little time here testing, while GPS powers up:
  testNeopixels();
#endif // HAS_NEOPIXELS

}

// ================================================================
// red an blue LEDs on pins D12, D15:

const int Red_LED_Pin = 12;     // Error LED
const int Blue_LED_Pin = 15;    // Connect LED

void setupLeds()
{
  pinMode(Red_LED_Pin, OUTPUT);
  pinMode(Blue_LED_Pin, OUTPUT);
  
  errorLed(false);
  commLed(false);
}

void errorLed(bool _on)
{
  digitalWrite(Red_LED_Pin, _on);
}

void commLed(bool _on)
{
  digitalWrite(Blue_LED_Pin, _on);
}


// ===================================================================
// different states are displayed on Serial, LEDs, LCD and Neopixels:

void displayWifiConnecting(int tries)
{
  char traceBuffer[512];

  Serial.print(tries);
  Serial.print("...");
#ifdef HAS_NEOPIXELS
  stripPixelOn(PIX_WIFI, tries % 2 == 0 ? pixColorYellow() : pixColorRed());
#endif // HAS_NEOPIXELS
  snprintf(traceBuffer, sizeof(traceBuffer), "WiFi: %s", mySsid());
  textLcdLine1(traceBuffer);
  snprintf(traceBuffer, sizeof(traceBuffer), "Try %d", tries);
  textLcdLine2(traceBuffer);
}

void displayWifiConnected()
{
#ifdef HAS_NEOPIXELS
  stripPixelOn(PIX_WIFI, pixColorGreen());
#endif //HAS_NEOPIXELS
}

void displayStateGettingRtcm()
{
  textLcdLine1("Getting RTCM");
}

bool lastHadRtcmData = false;
unsigned int tmpCnt = 0;

void displayHasRtcmData(bool hasData, bool isNoDataNormal)
{
#ifdef HAS_NEOPIXELS
  // this function is called often, avoid bothering the strip too much:
  if(hasData != lastHadRtcmData || (++tmpCnt % 100) == 0)
  {
    // in WORKING state short absence of RTCMs is normal, so we blink Blue/Green:
    stripPixelOn(PIX_RTCMS, hasData ? pixColorGreen() : (isNoDataNormal ? pixColorBlue() : pixColorRed()));
    lastHadRtcmData = hasData;
  }
#endif // HAS_NEOPIXELS
}

void displayStateConnectingToWifi()
{
  textLcdLine1("Connecting WiFi");
}

void displayStateConnectingToCaster()
{
  textLcdLine1("Connecting Caster");
#ifdef HAS_NEOPIXELS
  stripPixelOn(PIX_CASTER, pixColorYellow());
#endif // HAS_NEOPIXELS
}

void displayCasterIsConnected(bool isConnected)
{
#ifdef HAS_NEOPIXELS
  stripPixelOn(PIX_CASTER, isConnected ? pixColorGreen() : pixColorRed());
#endif // HAS_NEOPIXELS
}

bool runner = true;

void displayStateWorking(unsigned long nBytes)
{
  snprintf(traceBuffer, sizeof(traceBuffer), runner ? "Wrk   %d/%d/%d" : "Wrk...%d/%d/%d", connectionAttempts.size(), cntErrsReceivedRTCM_ms, cntErrsSentRTCM_ms);
  runner = !runner;
  textLcdLine1(traceBuffer);
  if(nBytes > 10485760)
  {
    // after 10MB sent, switch to displaying kilobytes:
    snprintf(traceBuffer, sizeof(traceBuffer), "KB: %d", nBytes / 1024);
  }
  else
  {
    snprintf(traceBuffer, sizeof(traceBuffer), "Bytes:%d", nBytes);
  }
  textLcdLine2(traceBuffer);
  errorLed(false);
}

void displayWorkDataIsFlowing(bool isFlowing)
{
#ifdef HAS_NEOPIXELS
  stripPixelOn(PIX_FLOW, isFlowing ? pixColorBlueMax() : pixColorGreen());
#endif // HAS_NEOPIXELS
}

void displayStateOnBreakNoRtcm()
{
  textLcdLine1("On Break:No RTCM");
  errorLed(true);
  commLed(false);
#ifdef HAS_NEOPIXELS
  stripAllPixelsOff();
#endif // HAS_NEOPIXELS
}

void displayStateDoghouse()
{
  textLcdLine1("In Doghouse");
  errorLed(true);
  commLed(false);
#ifdef HAS_NEOPIXELS
  stripAllPixelsOff();
#endif // HAS_NEOPIXELS
}

void displayStateIdle()
{
  textLcdLine1("Idle Switch ON");
  errorLed(false);
  commLed(false);
#ifdef HAS_NEOPIXELS
  stripAllPixelsOff();
#endif // HAS_NEOPIXELS
}

void displayNoGps()
{
  textLcdLine1("No GPS Detected");
  textLcdLine2("SDA=D21,SCL=D22");
  errorLed(true);
  commLed(false);
#ifdef HAS_NEOPIXELS
  stripPixelOn(PIX_RTCMS, pixColorRed());
#endif // HAS_NEOPIXELS
}

void displayGpsSetupError()
{
  textLcdLine1("GPS Setup Error");
  textLcdLine2("have uBlox F9P?");
  errorLed(true);
  commLed(false);
#ifdef HAS_NEOPIXELS
  stripPixelOn(PIX_RTCMS, pixColorRed());
#endif // HAS_NEOPIXELS
  delay(100);
}

void displayWaitAfterFailedConnection(int  m)
{
  snprintf(traceBuffer, sizeof(traceBuffer), "%d minutes left", m);
  textLcdLine1(traceBuffer);
  textLcdLine2("Failed to connect");
  errorLed(true);
  commLed(false);
#ifdef HAS_NEOPIXELS
  stripAllPixelsOff();
#endif // HAS_NEOPIXELS
}

void displayFatalError()
{
  Error("Fatal Error state");
  errorLed(true);
  commLed(false);
#ifdef HAS_NEOPIXELS
  stripAllPixelsOff();
#endif // HAS_NEOPIXELS
  for(;;) // Don't proceed, loop forever
  {
    textLcdLine1("Fatal Error");
    delay(100);
  }
}
