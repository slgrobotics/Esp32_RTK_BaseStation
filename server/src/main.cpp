
// This is heavily modified example NTRIP Server from SparkFun GNSS Library V2.0, u-Blox F9P section.
//
// The idea is to:
//    - let the station work hands-free forever
//    - without USB connection to laptop
//    - handle power outages, restore to normal operation
//    - handle network and Caster glitches
//    - avoid being banned by Caster for idle or numerous connections
//    - provide ample visual indication about the station's status
//
//
// This code is free (as free beer) - no guarantees or liability of any kind. See MIT license below. Use at your own risk.
// Send your suggestions to slg@quakemap.com  - beer accepted at https://www.paypal.com/paypalme/vitalbytes
// Please support SparkFun.com - buy their awesome products!
//
// Thank you for your interest,
// -- Sergei Grichine

// please set YOUR base station coordinates, see GNSS.ino tab around line 120
// see "secrets.h" tab for credentials, set yours - for network and Caster

// Permissive MIT Open Source license below:

/*
Copyright (c) 2021 Sergei Grichine

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 * Original SparkFun header below:
 *
  Note: compiles OK with v2.0 but is currently untested

  Use ESP32 WiFi to push RTCM data to RTK2Go (caster) as a Server
  By: SparkFun Electronics / Nathan Seidle
  Date: December 14th, 2020
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to gather RTCM data over I2C and push it to a casting service over WiFi.
  It's confusing, but the Arduino is acting as a 'server' to a 'caster'. In this case we will
  use RTK2Go.com as our caster because it is free. A rover (car, surveyor stick, etc) can
  then connect to RTK2Go as a 'client' and get the RTCM data it needs.

  You will need to register your mountpoint here: http://www.rtk2go.com/new-reservation/
  (They'll probably block the credentials we include in this example)

  To see if your mountpoint is active go here: http://rtk2go.com:2101/

  This is a proof of concept. Serving RTCM to a caster over WiFi is useful when you need to
  set up a high-precision base station.

  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  ZED-F9P RTK2: https://www.sparkfun.com/products/16481
  RTK Surveyor: https://www.sparkfun.com/products/17369

  Hardware Connections:
  Plug a Qwiic cable into the GNSS and a ESP32 Thing Plus
  If you don't have a platform with a Qwiic connection use the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
  Open the serial monitor at 115200 baud to see the output
*/

// =======================================================================================
//
// this is optional hardware, four neopixels on pin 26, using 3.3V<->5V level shifter:
// #define HAS_NEOPIXELS

// put your network and caster credentials here:
#include <Arduino.h>
#include <WiFi.h>

#include "Tracing.h"
#include "secrets.h"
#include "NTRIPManager.h"
#include "NetworkManager.h"
#include "GNSSManager.h"

// see https://github.com/rlogiacco/CircularBuffer
//     https://www.arduino.cc/reference/en/libraries/circularbuffer/
#define CIRCULAR_BUFFER_INT_SAFE

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Basic Connection settings to RTK2Go NTRIP Caster - See secrets.h for mount specific credentials
//
// const uint16_t casterPort = 2101;
// const char *casterHost = "rtk2go.com";

NTRIPManager::Caster ntripCaster;
void GNSSManager::onReceivedRTCM(uint8_t incoming)
{
  ntripCaster.send(incoming);
}
GNSSManager gnssManager;
NetworkManager networkManager;

// void SFE_UBLOX_GNSS::processRTCM(uint8_t incoming)
// {
//   gnssManager.processRTCM(incoming);
//   ntripCaster.send(incoming);
// }
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup()
{
  Serial.begin(115200); // Just for tracing on USB Serial, if connected. No harm if not.

  Trace("ELyir NTRIP Server | A SparkFun u-blox Example Reworked for ESP32");
  networkManager.setup(ssid, password);
  gnssManager.setup();
  ntripCaster.setup(casterPort, casterHost, ntrip_server_name, mountPoint, mountPointPW);
}

void loop()
{
  networkManager.work();
  gnssManager.work();
  ntripCaster.work(networkManager.isConnected());
}
