
/* ==============================================================================================
 *  
 *  Note: we use SparkFun u-blox ZED-F9P breakboard here, and that's the only one we tested with.
 *        https://www.sparkfun.com/products/16481
 *  
 *  DO NOT share I2C (Qwiic) bus with any other device. Specifically, OLED proved not to work.
 *  
 * ============================================================================================== */

#include <Wire.h> // Needed for I2C to GNSS. Wiring: Qwiic to SDA=D21,SCL=D22.
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> // http://librarymanager/All#SparkFun_u-blox_GNSS

SFE_UBLOX_GNSS myGNSS;

// This function gets called from the SparkFun u-blox Arduino Library.
// As each RTCM byte comes in you can specify what to do with it
// Useful for passing the RTCM correction data to a radio, Ntrip broadcaster, etc.
void SFE_UBLOX_GNSS::processRTCM(uint8_t incoming)
{
  lastReceivedRTCM_ms = millis();

  if(!rtcms.isFull())
  {
    rtcms.unshift(incoming);
  }
}

// call this often to let GNSS library work:
void workGnss()
{
  myGNSS.checkUblox(); // See if new data is available. Process RTCM bytes as they come in, fill "rtcms" FIFO.
}

void setupGNSS()
{
  Wire.begin();

  // when GPS and ESP32 are powered up together, give GPS time to boot up first:  
  delay(1000);

  //myGNSS.enableDebugging(); // Uncomment this line to enable debug messages

  while (myGNSS.begin() == false) // Connect to the u-blox module using Wire (I2C) port
  {
    Error("u-blox GNSS not detected at default I2C address. Please check wiring: Qwiic to SDA=D21,SCL=D22.");
    displayNoGps();
    delay(1000);
  }
  
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
    displayGpsSetupError();
    while (1);
  }
  else
    Trace("OK: GPS NMEA on I2C disabled");

  // Enable necessary RTCM sentences
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1005, COM_PORT_I2C, 1); // Enable message 1005 to output through UART2, message every second
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1074, COM_PORT_I2C, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1084, COM_PORT_I2C, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1094, COM_PORT_I2C, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1124, COM_PORT_I2C, 1);
  response &= myGNSS.enableRTCMmessage(UBX_RTCM_1230, COM_PORT_I2C, 10); // Enable message every 10 seconds

  if (response == false)
  {
    Error("Failed to enable GPS RTCM on I2C. Freezing...");
    displayGpsSetupError();
    while (1);
  }
  else
    Trace("OK: GPS RTCM sentences on I2C enabled");

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
  response &= myGNSS.setStaticPosition(34388053, 81, -533016706, -56, 347445856, 62); // With high precision 0.1mm parts


  if (response == false)
  {
    Error("Failed to set Base Station static position via I2C. Freezing...");
    displayGpsSetupError();
    while (1);
  }
  else
    Trace("OK: GPS Base Station static position set");

  // You could instead do a survey-in but it takes much longer to start generating RTCM data. See Example4_BaseWithLCD
  // Note: you have to change one of the parameters below to force F9P to go into survey again. Or it will use previous survey.
  //myGNSS.enableSurveyMode(60+-random, 5.000); // Enable Survey in, 60 seconds, 5.0m

  if (myGNSS.saveConfiguration() == false) // Save the current settings to flash and BBR
    Trace("Warn: GPS Module failed to save configuration to flash and BBR.");

  Trace("OK: GPS Module configuration complete");
}
