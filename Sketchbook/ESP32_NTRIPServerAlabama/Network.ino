
#include <WiFi.h>

// put your network and caster credentials here:
#include "secrets.h"

WiFiClient client;

bool setupWifiNetwork()
{
  Trace("IP: Connecting to local WiFi");
  
  WiFi.begin(ssid, password);

  int tries = 0;

  while (!isWifiConnected() && ++tries <= 10) {
    displayWifiConnecting(tries);
    delay(2000);
  }

  if(isWifiConnected())
  {
    char traceBuffer[512];

    snprintf(traceBuffer, sizeof(traceBuffer), "\nOK: WiFi connected, local IP: ");
    Serial.print(traceBuffer);
    Serial.print(WiFi.localIP());
    return true;
  }

  Error("Failed to connect to WiFi");
  return false;
}

const char *mySsid()
{
  return ssid;
}

void closeCasterConnection()
{
  Trace("IP: closing Caster connection");
  client.stop();
  displayCasterIsConnected(false);
}

bool isWifiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

/*
 * If not connected to Caster - tries to connect
 * 
 * Returns:
 *  0  - No need to connect, already connected
 *  1  - Connected successfully
 *  10 - Failed to open socket at casterHost, casterPort (wrong address?)
 *  11 - Failed to get caster response within 5 seconds
 *  12 - Caster responded, but not with expected "200"
 *  
 */
byte tryConnectToCaster()
{
  if (client.connected())
  {
    return 0; // No need to connect, already connected
  }
  
  snprintf(traceBuffer, sizeof(traceBuffer), "IP: Opening socket to Caster: %s:%d\n", casterHost, casterPort);
  Trace(traceBuffer);

  if (!client.connect(casterHost, casterPort)) // Attempt connection
  {
    Error("Connection to caster socket failed");
    return 10;  // Failed to open socket at casterHost, casterPort (wrong address?)
  }
  
  Trace("OK: Connected to Caster socket");

  // log connection attempt, for possibility of being sent to doghouse:
  unsigned long connTime = millis();
  connectionAttempts.unshift(connTime);

  // prepare Caster credentials:
  const int SERVER_BUFFER_SIZE  = 512;
  char serverBuffer[SERVER_BUFFER_SIZE];

  snprintf(serverBuffer, SERVER_BUFFER_SIZE, "SOURCE %s /%s\r\nSource-Agent: NTRIP %s/%s\r\n\r\n",
           mntpnt_pw, mntpnt, ntrip_server_name, "App Version 1.0");

  snprintf(traceBuffer, sizeof(traceBuffer), "IP: Attempt: %d - Sending Caster credentials:\n%s\n", connectionAttempts.size(), serverBuffer);
  Trace(traceBuffer);
  
  // send Caster credentials:
  client.write(serverBuffer, strlen(serverBuffer));

  // Wait for response, but not too long:
  while (client.available() == 0)    // any bytes available from Caster?
  {
    if (millis() - connTime > 5000)
    {
      Error(" >>> Caster connection timeout !");
      closeCasterConnection();    // that's client.stop() + display
      return 11;  // Failed to get caster response within 5 seconds
    }
    delay(10);
  }

  // Caster responded promptly - check reply for "200":
  bool connectionSuccess = false;
  char responseBuffer[512];
  int responseSpot = 0;
  while (client.available())    // bytes available
  {
    responseBuffer[responseSpot++] = client.read(); // one byte at a time
    
    if (strstr(responseBuffer, "200") > 0) // Look for 'ICY 200 OK'
    {
      connectionSuccess = true;            // Got it!
    }
    
    // we read whole response, but not over 511 bytes long:
    if (responseSpot == sizeof(responseBuffer) - 1) 
      break;
  }
  responseBuffer[responseSpot] = '\0';

  if (!connectionSuccess)
  {
    closeCasterConnection();    // that's client.stop() + display
    snprintf(traceBuffer, sizeof(traceBuffer), "Failed to connect to RTK2Go: %s", responseBuffer);
    Error(traceBuffer);
    return 12;  // Caster responded, but not with expected "200"
  }

  return 1; // signal successful new connection
}

/*
 * Trying to send all bytes from rtcm FIFO queue to Caster's stream
 * 
 * Returns:
 *  0  - all good, no data in queue
 *  1  - had data, sent successfully
 *  10 - client disconnected
 *  11 - idle switch up
 * 
 */
int tryServe()
{
  // processRTCM() fills the FIFO queue. We empty it here.

  int didSend = 0;
  
  while(!rtcms.isEmpty() && client.connected())
  {
    if (isIdleSwitchOn()) // switch up prevents sending, closes connection.
    {
      closeCasterConnection();
      return 11;
    }
    
    byte outgoing = rtcms.pop();

    client.write(outgoing); // Send this byte to socket
    serverBytesSent++;
    didSend = 1;

    lastSentRTCM_ms = millis();
  }

  return client.connected() ? didSend : 10;
}
