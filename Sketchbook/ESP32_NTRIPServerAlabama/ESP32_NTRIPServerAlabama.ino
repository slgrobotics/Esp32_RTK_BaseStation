
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
#define HAS_NEOPIXELS

// see https://github.com/rlogiacco/CircularBuffer
//     https://www.arduino.cc/reference/en/libraries/circularbuffer/
#define CIRCULAR_BUFFER_INT_SAFE
#include <CircularBuffer.h>

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Basic Connection settings to RTK2Go NTRIP Caster - See secrets.h for mount specific credentials
//
const uint16_t casterPort = 2101;
const char * casterHost = "rtk2go.com";
const char * ntrip_server_name = "SparkFun_RTK_Surveyor-SG";

// parameters that define timing of the State Machine:

const int reportInterval_ms = 1000*60*10;   // not too often, avoid scrolling important events

const int breakInterval_ms = 1000*60;       // hope that no-RTCM condition will be resolved in about a minute
const int maxTimeBeforeHangup_ms = 10000;   // If we fail to get a complete RTCM frame after 10s, then disconnect from caster
const int maxFailedCasterConnectionAttempts = 10;  // we retry at progressive intervals several times, then wait long to avoid IP ban
const int waitAfterFailedConnectionMinutes = 60;   // to wait really long after retries, to avoid being banned by the Caster

// all the variables we need globally:

unsigned long lastReceivedRTCM_ms = 0;      // Time of last data received from GPS
unsigned long lastSentRTCM_ms = 0;          // Time of last data pushed to socket
unsigned long lastWentToBreak = 0;          // when went to break for no RTCM
unsigned long lastReport_ms = 0;            // Time of last report of bytes sent to Serial

unsigned long serverBytesSent = 0;          // Just a running total, on ESP32 4 bytes, max 4,294,967,295 (4GB)
                                            // at 20MB/day it takes ~200 days to rollover

// have a volatile FIFO to buffer RCTMs. GNSS library is unlikely to use interrupts unsafely, but still:
CircularBuffer<byte,2048> rtcms;    

// Have a FIFO to log connection attempts, successful or not. Logs time in ms.
// When this queue is overfilled (after 512 attempts in 24 hours), we'll wait in doghouse.
// Older than 24 hours records are cleared:
CircularBuffer<unsigned long,512> connectionAttempts;    

unsigned int failedCasterConnectionAttempts = 0;  // for Caster fast-reconnecting timing

char traceBuffer[512];

unsigned long _now; // more or less current millis()

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup()
{
  Serial.begin(115200); // Just for tracing on USB Serial, if connected. No harm if not.

  rtcms.clear();
  connectionAttempts.clear();

  _now = millis();  // rollover in 49 days. Great discussion at https://forum.arduino.cc/index.php?topic=543895.0
                    // "When using subtraction on unsigned integers. the subtraction will automatically handle it."

  // no need to use F macro on ESP32, plenty of RAM
  // F keeps a string inside of PROGMEM and not allow it to consume RAM
  // see https://www.baldengineer.com/arduino-f-macro.html
  //Serial.println(F("SparkFun u-blox Example - NTRIP Server"));
  Trace("SparkFun u-blox Example Reworked - NTRIP Server Alabama");

  setupDisplays();

  setupControls();

  setupGNSS();

  lastReport_ms = millis();
  lastReceivedRTCM_ms = millis() - 100000;  // make sure we don't think we have RTCMs at start
  lastSentRTCM_ms = millis() - 100000;
}

enum States {
              STATE_NONE,
              STATE_GETTING_RTCM,
              STATE_CONNECTING_TO_WIFI,
              STATE_CONNECTING_TO_CASTER,
              STATE_WORKING,
              STATE_BREAK_FOR_NO_RTCM,
              STATE_IN_DOGHOUSE,
              STATE_IDLE_BY_SWITCH,
              STATE_FATAL_ERROR
};

const char* StateStrings[] = { "NONE", "GETTING_RTCM", "CONNECTING_TO_WIFI", "CONNECTING_TO_CASTER",
                          "WORKING", "BREAK_FOR_NO_RTCM", "IN_DOGHOUSE", "IDLE_BY_SWITCH", "FATAL_ERROR" };

States state = STATE_IDLE_BY_SWITCH;
States state_prev = STATE_NONE;

void loop()
{
//  testNeopixels();
//  delay(1000);
//  return;
  
  _now = millis();
  
  // Print state change, if any:
  if(state != state_prev)
  {
    snprintf(traceBuffer, sizeof(traceBuffer), "STATE: %s --> %s", StateStrings[(int)state_prev], StateStrings[(int)state]);
    Trace(traceBuffer);
    state_prev = state;
  }
  
  // ======= first do some chores: =====================
  
  // calling this often to let GNSS library work:
  workGnss();

  // check controls often:
  if(isIdleSwitchOn())
  {
    if(state != STATE_IDLE_BY_SWITCH)
    {
      // switch up prevents sending, closes caster connection. Network remains.
      closeCasterConnection();
      rtcms.clear();
      state = STATE_IDLE_BY_SWITCH;
    }
  }
  else if (state != STATE_GETTING_RTCM && !isWifiConnected())
  {
    state = STATE_CONNECTING_TO_WIFI;
  }

  // we come here VERY often, unless there is a delay() in a state handler

  // ======= now, work the state machine: ================

  workTheStateMachine();
  
  // do some low priority/rate chores:

  _now = millis();
  
  // friendly blinking of LEDs:
  displayHasRtcmData(_now - lastReceivedRTCM_ms < 2000, state == STATE_WORKING);

  // Report some statistics:
  if (_now - lastReport_ms > reportInterval_ms)
  {
    lastReport_ms += reportInterval_ms;
    snprintf(traceBuffer, sizeof(traceBuffer), "FYI: STATE: %s  Minutes up: %d  Total sent: %d  connectionAttempts: %d\n",
                                                StateStrings[state], _now / 60000, serverBytesSent, connectionAttempts.size());
    Trace(traceBuffer);
  }

  // purge connection attempts records that are older than 24 hours:
  if(!purgeConnectionAttempts())
  {
    // after purging, the queue is still full. We were really bad in the last 24 hours.
    // if too many attempts to connect to caster in 24 hours, we have to disconnect and wait "in doghouse":
    state = STATE_IN_DOGHOUSE;
  }

  // back in loop
}

bool purgeConnectionAttempts()
{
  // purge connection attempts records that are older than 24 hours:
  if(!connectionAttempts.isEmpty())
  {
    // clear one, we'll come here very soon for more, if any:
    if(millis() - connectionAttempts.last() > 24*3600000)
    {
       connectionAttempts.pop();
       Trace("FYI: purged one 24-hour old connection attempt record");
    }

    if(connectionAttempts.isFull())
    {
      // after purging, the queue is still full.
      // if too many attempts to connect to caster in 24 hours,
      // we have to disconnect and wait "in doghouse":
      return false;
    }
  }
  return true; // queue in normal state, no problem
}
