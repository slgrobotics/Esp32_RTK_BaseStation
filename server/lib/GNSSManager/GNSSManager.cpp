
/* ==============================================================================================
 *
 *  Note: we use SparkFun u-blox ZED-F9P breakboard here, and that's the only one we tested with.
 *        https://www.sparkfun.com/products/16481
 *
 *  DO NOT share I2C (Qwiic) bus with any other device. Specifically, OLED proved not to work.
 *
 * ============================================================================================== */

#include "GNSSManager.h"

#include <Wire.h> // Needed for I2C to GNSS. Wiring: Qwiic to SDA=D21,SCL=D22.

SFE_UBLOX_GNSS myGNSS;

void SFE_UBLOX_GNSS::processRTCM(uint8_t incoming)
{
  // GNSSManager::_processRTCM(incoming);
  GNSSManager::onReceivedRTCM(incoming);
}
// void GNSSManager::_processRTCM(byte incoming)
// {
//   // GNSSManager::lastReceivedRTCM_ms = millis();
//   GNSSManager::onReceivedRTCM(incoming);
//   // if (myGNSS.rtcmFrameCounter % 16 == 0)
//   //   Serial.println();
//   // Serial.print(F(" "));
//   // if (incoming < 0x10)
//   //   Serial.print(F("0"));
//   // Serial.print(incoming, HEX);
// }
void GNSSManager::onReceivedRTCM(byte incoming)
{
  byte k = incoming;
  k += 0;
}
// call this often to let GNSS library work:
void GNSSManager::work()
{
  myGNSS.checkUblox(); // See if new data is available. Process RTCM bytes as they come in, fill "rtcms" FIFO.
  // displayStateGettingRtcm();
}
// This function gets called from the SparkFun u-blox Arduino Library.
// As each RTCM byte comes in you can specify what to do with it
// Useful for passing the RTCM correction data to a radio, Ntrip broadcaster, etc.

bool GNSSManager::_startAsStaticPositionMode()
{

  myGNSS.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA | COM_TYPE_RTCM3); // UBX+RTCM3 is not a valid option so we enable all three.

  myGNSS.setNavigationFrequency(1); // Set output in Hz. RTCM rarely benefits from >1Hz.

  // Disable all NMEA sentences
  bool response = true;
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_GGA, COM_PORT_I2C);
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_GSA, COM_PORT_I2C);
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_GSV, COM_PORT_I2C);
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_RMC, COM_PORT_I2C);
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_GST, COM_PORT_I2C);
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_GLL, COM_PORT_I2C);
  response &= myGNSS.disableNMEAMessage(UBX_NMEA_VTG, COM_PORT_I2C);

  if (response == false)
  {
    Error("Failed to disable GPS NMEA on I2C. Freezing...");
    // displayGpsSetupError();
    while (1)
      ;
  }
  else
    Trace("OK: GPS NMEA on I2C disabled");

  // Enable necessary RTCM sentences. See ...\libraries\SparkFun_u-blox_GNSS_Arduino_Library\src\SparkFun_u-blox_GNSS_Arduino_Library.h
  // Some commented out aren't supported by SparkFun library. The shorter MSM4 10**4's are replaced by MSM7 10**7 higher precision ones.

  // response &= myGNSS.enableRTCMmessage(UBX_RTCM_1004, COM_PORT_I2C, 1);    // Enable message 1004 to output through I2C, message every second
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1005, COM_PORT_I2C, 10);   // Stationary RTK reference ARP. Enable to output through I2C, message every 10 seconds
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1005, COM_PORT_UART2, 10); // Same for UART2, where HolyBro Telemetry Radio is connected for local support
  // response &= myGNSS.enableRTCMmessage(UBX_RTCM_1033, COM_PORT_I2C, 10);   // GPS module and antenna descriptor, every 10 seconds
  // response &= myGNSS.enableRTCMmessage(UBX_RTCM_1074, COM_PORT_I2C, 1);    // GPS MSM4
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1077, COM_PORT_I2C, 1); // GPS MSM7
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1077, COM_PORT_UART2, 1);
  // response &= myGNSS.enableRTCMmessage(UBX_RTCM_1084, COM_PORT_I2C, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1087, COM_PORT_I2C, 1); // GLONASS MSM7
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1087, COM_PORT_UART2, 1);
  // response &= myGNSS.enableRTCMmessage(UBX_RTCM_1094, COM_PORT_I2C, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1097, COM_PORT_I2C, 1); // Galileo MSM7
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1097, COM_PORT_UART2, 1);
  // response &= myGNSS.enableRTCMmessage(UBX_RTCM_1124, COM_PORT_I2C, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1127, COM_PORT_I2C, 1); // BeiDou MSM7
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1127, COM_PORT_UART2, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1230, COM_PORT_I2C, 10); // GLONASS code-phase biases. Enable message every 10 seconds
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1230, COM_PORT_UART2, 10);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_4072_0, COM_PORT_I2C, 1); // Reference station PVT (uBlox proprietary RTCM message)
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_4072_0, COM_PORT_UART2, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_4072_1, COM_PORT_I2C, 1); // Additional reference station information (ublox proprietary RTCM message)
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_4072_1, COM_PORT_UART2, 1);

  if (response == false)
  {
    Error("Failed to enable GPS RTCM on I2C. Freezing...");
    // displayGpsSetupError();
    while (1)
      ;
  }
  else
    Trace("OK: GPS RTCM sentences on I2C and UART2 enabled");

  // -1280208.308,-4716803.847,4086665.811 is SparkFun HQ so...
  // Units are cm with a high precision extension so -1234.5678 should be called: (-123456, -78)
  // For more infomation see Example12_setStaticPosition
  // Note: If you leave these coordinates in place and setup your antenna *not* at SparkFun, your receiver
  // will be very confused and fail to generate correction data because, well, you aren't at SparkFun...
  // See this tutorial on getting PPP coordinates: https://learn.sparkfun.com/tutorials/how-to-build-a-diy-gnss-reference-station/all
  // response &= myGNSS.setStaticPosition(-128020830, -80, -471680384, -70, 408666581, 10); // With high precision 0.1mm parts
  //                                     (-1280.208.30, -80, -4716.803.84, -70, 4086.665.81, 10); // dotted km, for readability

  // SparkFun HQ in Denver, CO. approximately:
  //  WGS84 ECEF-X (km)  -1279.896
  //  WGS84 ECEF-Y (km) -4715.651
  //  WGS84 ECEF-Z (km) 4085.661
  //  WGS84 Latitude (d:m:s)  40:5:25.08501
  //  WGS84 Longitude (d:m:s) -105:11:6.31424
  //  WGS84 Altitude (m)  0.000

  // My approximate coordinates, from https://dominoc925-pages.appspot.com/mapplets/cs_ecef.html
  //  WGS84 ECEF-X (km)  343.865
  //  WGS84 ECEF-Y (km) -5330.070
  //  WGS84 ECEF-Z (km) 3474.390
  //  WGS84 Latitude (d:m:s)  33:13:11.74748
  //  WGS84 Longitude (d:m:s) -86:18:31.42565
  //  WGS84 Altitude (m)  0.000

  //  After 24-hour survey Feb 23, 2021:
  //   -86.308626946           Mean ECEF x:   343880.5381
  //    33.219968128                     y: -5330167.0656
  //   119.3327 m alt (ellipsoid)        z:  3474458.5662
  //        Mean StdDev:        0.0530 m
  //        Precision:  Horiz: 0.0433  Vert: 0.0306

  //        Latitude:  38.6027534
  //        Longitude: -92.1239043
  response &= myGNSS.setStaticPosition(34388053, 81, -533016706, -56, 347445856, 62); // With high precision 0.1mm parts

  if (response == false)
  {
    Error("Failed to set Base Station static position via I2C. Freezing...");
    // displayGpsSetupError();
    while (1)
      ;
  }
  else
    Trace("OK: GPS Base Station static position set");
  return true;
}
bool GNSSManager::_startAsSurveyInMode()
{
  myGNSS.setI2COutput(COM_TYPE_UBX); // Set the I2C port to output UBX only (turn off NMEA noise)
  bool response = true;
  // Check if Survey is in Progress before initiating one
  //  From v2.0, the data from getSurveyStatus (UBX-NAV-SVIN) is returned in UBX_NAV_SVIN_t packetUBXNAVSVIN
  //  Please see u-blox_structs.h for the full definition of UBX_NAV_SVIN_t
  //  You can either read the data from packetUBXNAVSVIN directly
  //  or can use the helper functions: getSurveyInActive; getSurveyInValid; getSurveyInObservationTime; and getSurveyInMeanAccuracy
  response = myGNSS.getSurveyStatus(2000); // Query module for SVIN status with 2000ms timeout (request can take a long time)

  if (response == false) // Check if fresh data was received
  {
    Error("Failed to get Survey In status");
    while (1)
      ; // Freeze
  }

  if (myGNSS.getSurveyInActive() == true) // Use the helper function
  // if (myGNSS.packetUBXNAVSVIN->data.active > 0) // Or we could read active directly
  {
    Trace("IP: Survey already in progress.");
  }
  else
  {
    // Start survey
    // The ZED-F9P is slightly different than the NEO-M8P. See the Integration manual 3.5.8 for more info.
    // response = myGNSS.enableSurveyMode(300, 2.000); //Enable Survey in on NEO-M8P, 300 seconds, 2.0m
    response = myGNSS.enableSurveyMode(60, 5.000); // Enable Survey in, 60 seconds, 5.0m
    // response = myGNSS.enableSurveyModeFull(86400, 2.000); //Enable Survey in, 24 hours, 2.0m
    if (response == false)
    {
      Error("Survey start failed. Freezing...");
      while (1)
        ;
    }
    Trace("IP: Survey started. This will run until 60s has passed and less than 5m accuracy is achieved.");
  }

  // while (Serial.available())
  //   Serial.read(); // Clear buffer

  // Begin waiting for survey to complete
  while (myGNSS.getSurveyInValid() == false) // Call the helper function
  // while (myGNSS.packetUBXNAVSVIN->data.valid == 0) // Or we could read valid directly
  {
    if (Serial.available())
    {
      byte incoming = Serial.read();
      if (incoming == 'x')
      {
        // Stop survey mode
        response = myGNSS.disableSurveyMode(); // Disable survey
        Trace("OK: Survey stopped");
        break;
      }
    }

    // From v2.0, the data from getSurveyStatus (UBX-NAV-SVIN) is returned in UBX_NAV_SVIN_t packetUBXNAVSVIN
    // Please see u-blox_structs.h for the full definition of UBX_NAV_SVIN_t
    // You can either read the data from packetUBXNAVSVIN directly
    // or can use the helper functions: getSurveyInActive; getSurveyInValid; getSurveyInObservationTime; getSurveyInObservationTimeFull; and getSurveyInMeanAccuracy
    response = myGNSS.getSurveyStatus(2000); // Query module for SVIN status with 2000ms timeout (req can take a long time)

    if (response == true) // Check if fresh data was received
    {
      Serial.print(F("Press x to end survey - "));
      Serial.print(F("Time elapsed: "));
      Serial.print((String)myGNSS.getSurveyInObservationTimeFull()); // Call the helper function
      Serial.print(F(" ("));
      Serial.print((String)myGNSS.packetUBXNAVSVIN->data.dur); // Read the survey-in duration directly from packetUBXNAVSVIN

      Serial.print(F(") Accuracy: "));
      Serial.print((String)myGNSS.getSurveyInMeanAccuracy()); // Call the helper function
      Serial.print(F(" ("));
      // Read the mean accuracy directly from packetUBXNAVSVIN and manually convert from mm*0.1 to m
      float meanAcc = ((float)myGNSS.packetUBXNAVSVIN->data.meanAcc) / 10000.0;
      Serial.print((String)meanAcc);
      Serial.println(F(")"));
    }
    else
    {
      Serial.println(F("SVIN request failed"));
    }

    delay(1000);
  }
  Serial.println(F("Survey valid!"));

  Serial.println(F("Base survey complete! RTCM now broadcasting."));
  myGNSS.setI2COutput(COM_TYPE_UBX | COM_TYPE_NMEA | COM_TYPE_RTCM3); // UBX+RTCM3 is not a valid option so we enable all three.
  // myGNSS.setI2COutput(COM_TYPE_UBX | COM_TYPE_RTCM3);                 // Set the I2C port to output UBX and RTCM sentences (not really an option, turns on NMEA as well)
  return true;
}

void GNSSManager::setup()
{

  Wire.begin();
  // GNSSManager::lastReceivedRTCM_ms = 0;
  // GNSSManager::myGNSS;
  // when GPS and ESP32 are powered up together, give GPS time to boot up first:
  delay(1000);

  // myGNSS.enableDebugging(); // Uncomment this line to enable debug messages

  while (myGNSS.begin() == false) // Connect to the u-blox module using Wire (I2C) port
  {
    Error("u-blox GNSS not detected at default I2C address. Please check wiring: Qwiic to SDA=D21,SCL=D22.");
    // displayNoGps();
    delay(1000);
  }
#ifdef USE_SURVEY_IN_MODE
  GNSSManager::_startAsSurveyInMode();
#else
  GNSSManager::_startAsStaticPositionMode();
  // You could instead do a survey-in but it takes much longer to start generating RTCM data. See Example4_BaseWithLCD
  // Note: you have to change one of the parameters below to force F9P to go into survey again. Or it will use previous survey.
  // myGNSS.enableSurveyMode(60+-random, 5.000); // Enable Survey in, 60 seconds, 5.0m
#endif

  if (myGNSS.saveConfiguration() == false) // Save the current settings to flash and BBR
    Trace("Warn: GPS Module failed to save configuration to flash and BBR.");

  Trace("OK: GPS Module configuration complete");
}
