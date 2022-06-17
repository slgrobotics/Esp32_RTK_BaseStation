

#ifndef ELYIR_GNSS_MGR_H // Guard against multiple inclusion
#define ELYIR_GNSS_MGR_H

#define USE_SURVEY_IN_MODE

#include <Arduino.h>
#include "Tracing.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> // http://librarymanager/All#SparkFun_u-blox_GNSS

// class GNSS
// {
// public:
//     GNSS();
//     void setup();
//     void work();
//     void onReceivedRTCM(byte incoming);
// };

class GNSSManager
{

public:
    // call this often to let GNSS library work:

    // static SFE_UBLOX_GNSS myGNSS;
    static void work();
    static void setup();
    // static unsigned long lastReceivedRTCM_ms; // Time of last data received from GPS
    static void onReceivedRTCM(byte incoming) __attribute__((weak));

private:
    static void _processRTCM(byte incoming);
    static bool _startAsSurveyInMode();
    static bool _startAsStaticPositionMode();
    // GNSSManager() {}                     // Private so that it can't be called
    // GNSSManager(GNSSManager const &);    // Don't Implement
    // void operator=(GNSSManager const &); // Don't implement
};

#endif
