#ifndef ELYIR_DISPLAYS_H
#define ELYIR_DISPLAYS_H

#include "DisplayLCD.h"
#include "DisplayNeoPixel.h"

void setupDisplays();
void setupLeds();
void errorLed(bool _on);
void commLed(bool _on);
void displayWifiConnecting(int tries);
void displayWifiConnected();
void displayStateGettingRtcm();
void displayHasRtcmData(bool hasData, bool isNoDataNormal);
void displayStateConnectingToWifi();
void displayStateConnectingToCaster();
void displayCasterIsConnected(bool isConnected);
void displayStateWorking(unsigned long nBytes);
void displayWorkDataIsFlowing(bool isFlowing);
void displayStateOnBreakNoRtcm();
void displayStateDoghouse();
void displayStateIdle();
void displayNoGps();
void displayGpsSetupError();
void displayWaitAfterFailedConnection(int m);
void displayFatalError();

#endif