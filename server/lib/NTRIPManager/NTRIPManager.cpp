

#include "NTRIPManager.h"

void NTRIPManager::Caster::setup(uint16_t casterPort,
                                 const char *casterHost,
                                 const char *ntrip_server_name,
                                 const char *mntpnt,
                                 const char *mntpnt_pw)
{
    _casterPort = casterPort;
    _casterHost = casterHost;
    _ntrip_server_name = ntrip_server_name;
    _mntpnt = mntpnt;
    _mntpnt_pw = mntpnt_pw;
    lastSentRTCM_ms = millis() - 100000;
    lastReport_ms = millis();
    lastReceivedRTCM_ms = millis() - 100000; // make sure we don't think we have RTCMs at start
    lastWentToBreak = 0;
    lastRTCMWaitReport_ms = 0;
    cntErrsReceivedRTCM_ms = 0;
    cntErrsSentRTCM_ms = 0;
    buffer.clear();
    connectionAttempts.clear();
    serverBytesSent = 0;
    failedCasterConnectionAttempts = 0;
    connectionState = ConnectionStates::DISCONNECTED;
    // _client = client;
}
void NTRIPManager::Caster::send(uint8_t data)
{
    // Info("NTRIPManager: Recieved RTCM data");
    lastReceivedRTCM_ms = millis();
    if (!buffer.isFull())
    {
        buffer.unshift(data);
    }
}

NTRIPManager::ConnectionStates NTRIPManager::Caster::_connect()
{
    if (isConnectedAndAuthenticated())
    {
        connectionState = ConnectionStates::CONNECTED_AND_AUTHENTICATED;
        // Info("No need to connect, already connected");
        return ConnectionStates::CONNECTED_AND_AUTHENTICATED;
        // No need to connect, already connected
    }
    snprintf(traceBuffer, sizeof(traceBuffer), "IP: Opening socket to Caster: %s:%d\n", _casterHost, _casterPort);
    Trace(traceBuffer);

    if (!client.connect(_casterHost, _casterPort)) // Attempt connection
    {
        Error("Connection to caster socket failed");
        _onConnectionFailed();
        connectionState = ConnectionStates::FAILED_TO_OPEN_SOCKET;
        return FAILED_TO_OPEN_SOCKET; // Failed to open socket at casterHost, casterPort (wrong address?)
    }

    connectionState = ConnectionStates::CONNECTED_BUT_AUTHENTICATING;
    Trace("OK: Connected to Caster socket");

    // log connection attempt, for possibility of being sent to doghouse:
    unsigned long connTime = millis();
    connectionAttempts.unshift(connTime);

    // prepare Caster credentials:
    const int SERVER_BUFFER_SIZE = 512;
    char serverBuffer[SERVER_BUFFER_SIZE];

    snprintf(serverBuffer, SERVER_BUFFER_SIZE, "SOURCE %s /%s\r\nSource-Agent: NTRIP %s/%s\r\n\r\n",
             _mntpnt_pw, _mntpnt, _ntrip_server_name, "App Version 1.0");

    snprintf(traceBuffer, sizeof(traceBuffer), "IP: Attempt: %d - Sending Caster credentials:\n%s\n", connectionAttempts.size(), serverBuffer);
    Trace(traceBuffer);

    // send Caster credentials:
    client.write(serverBuffer, strlen(serverBuffer));

    // Wait for response, but not too long:
    while (client.available() == 0) // any bytes available from Caster?
    {
        if (millis() - connTime > 5000)
        {
            Error(" >>> Caster connection timeout !");
            close(); // that's client.stop() + display
            _onConnectionFailed();
            connectionState = ConnectionStates::FAILED_TO_GET_CASTER_RESPONSE;
            return FAILED_TO_GET_CASTER_RESPONSE; // Failed to get caster response within 5 seconds
        }
        delay(10);
    }

    // Caster responded promptly - check reply for "200":
    bool connectionSuccess = false;
    char responseBuffer[512];
    int responseSpot = 0;
    while (client.available()) // bytes available
    {
        responseBuffer[responseSpot++] = client.read(); // one byte at a time

        if (strstr(responseBuffer, "200") > 0) // Look for 'ICY 200 OK'
        {
            connectionSuccess = true; // Got it!
        }

        // we read whole response, but not over 511 bytes long:
        if (responseSpot == sizeof(responseBuffer) - 1)
            break;
    }
    responseBuffer[responseSpot] = '\0';

    if (!connectionSuccess)
    {
        close(); // that's client.stop() + display
        snprintf(traceBuffer, sizeof(traceBuffer), "Failed to connect to Caster Server: %s", responseBuffer);
        Error(traceBuffer);
        _onConnectionFailed();
        connectionState = ConnectionStates::RESPONSE_WAS_NOT_200;
        return RESPONSE_WAS_NOT_200; // Caster responded, but not with expected "200"
    }

    _onConnected();
    return CONNECTED_AND_AUTHENTICATED; // signal successful new connection
}

NTRIPManager::CloseStates NTRIPManager::Caster::close()
{
    Trace("Info: Attempting to close caster socket");
    if (!isConnected())
    {
        Trace("Info: No need to close, already closed");
        // onClosed(NOT_CONNECTED);
        return NOT_CONNECTED; // No need to close, already closed
    }
    client.stop();
    Trace("OK: Closed Caster socket");
    // displayCasterIsConnected(false);
    _onClosed();
    return CLOSED; // signal successful close
}

void NTRIPManager::Caster::work(bool isNetworkConnected)
{
    unsigned long _now = millis();
    // if (_now < waitTimeBeforeInitialConnection_ms)
    // {
    //     snprintf(traceBuffer, sizeof(traceBuffer), "Waiting for : %d before attempting initial caster connection.\n", (waitTimeBeforeInitialConnection_ms - _now) / 1000);
    //     Trace(traceBuffer);
    //     return;
    // }
    if (_noNewRTCMRecivedForAWhile())
    {
        if (_now - lastRTCMWaitReport_ms > rtcmWaithInterval_ms)
        {
            lastRTCMWaitReport_ms = _now;
            snprintf(traceBuffer, sizeof(traceBuffer), "Waiting for RTCM data before attempting initial caster connection.\n", (waitTimeBeforeInitialConnection_ms - _now) / 1000);
            Trace(traceBuffer);
        }
        return;
    }
    if (!isNetworkConnected)
    {
        return; // no network connection
    }
    // Close socket if we don't have new data for 10s
    // RTK2Go will ban your IP address if you abuse it. See http://www.rtk2go.com/how-to-get-your-ip-banned/
    // So let's not leave the socket open/hanging without data
    if (isConnectedAndAuthenticated())
    {
        if (_noNewRTCMRecivedForAWhile())
        {
            Error("RTCM timeout. Disconnecting...");
            snprintf(traceBuffer, sizeof(traceBuffer), "Now: %d,   Last Recv %d,   maxTimeBeforeHangup_ms: %d\n", _now, lastReceivedRTCM_ms, maxTimeBeforeHangup_ms);
            Trace(traceBuffer);
            close();
            cntErrsReceivedRTCM_ms++;
            lastWentToBreak = _now;
            return;
        }
        else if (_noRTCMSentForAWhile())
        {
            // this could be IP provider outage, while WiFi is on, Internet is not available:
            Error("Send to caster timeout. Disconnecting...");
            close();
            cntErrsSentRTCM_ms++;
            lastWentToBreak = _now;
            return;
        }
        // We didn't have RTCMs for 10 seconds, so we disconnected from caster and are waiting
        // hope that no-RTCM condition will be resolved in about a minute:
        else if (breakInterval_ms < (_now - lastWentToBreak))
        {
            return;
        }

        if (!purgeConnectionAttempts())
        {
            return;
        }
        // Report some statistics:
        if (_now - lastReport_ms > reportInterval_ms)
        {
            lastReport_ms += reportInterval_ms;
            snprintf(traceBuffer, sizeof(traceBuffer), "Minutes up: %d  Total sent: %d  connectionAttempts: %d\n", _now / 60000, serverBytesSent, connectionAttempts.size());
            Trace(traceBuffer);
        }
        _sendData();
    }
    else // not connected
    {
        _connect();
    }
}
void NTRIPManager::Caster::_sendData()
{
    while (!buffer.isEmpty() && isConnectedAndAuthenticated())
    {
        byte outgoing = buffer.pop();

        client.write(outgoing); // Send this byte to socket
        serverBytesSent++;
        lastSentRTCM_ms = millis();
    }
    // snprintf(traceBuffer, sizeof(traceBuffer), "NTRIPManager: Pushed RTCM data -- Total: %dBytes \n", serverBytesSent);
    // Trace(traceBuffer);
}

bool NTRIPManager::Caster::purgeConnectionAttempts()
{
    // purge connection attempts records that are older than 24 hours:
    if (!connectionAttempts.isEmpty())
    {
        // clear one, we'll come here very soon for more, if any:
        if (millis() - connectionAttempts.last() > 24 * 3600000)
        {
            connectionAttempts.pop();
            Trace("FYI: purged one 24-hour old connection attempt record");
        }

        if (connectionAttempts.isFull())
        {
            // after purging, the queue is still full.
            // if too many attempts to connect to caster in 24 hours,
            // we have to disconnect and wait "in doghouse":
            Trace("It seems we are in doghouse. Waiting...");
            return false;
        }
    }
    return true; // queue in normal state, no problem
}

void NTRIPManager::Caster::_onClosed()
{
    connectionState = ConnectionStates::DISCONNECTED;
    // displayCasterIsConnected(true);
    // we just connected, need to clear the buffer (which was possibly overfilled);
    Trace("OK: Closed connection to Caster socket");
    onClosed();
}
void NTRIPManager::Caster::_onConnected()
{
    failedCasterConnectionAttempts = 0;
    connectionState = ConnectionStates::CONNECTED_AND_AUTHENTICATED;
    // displayCasterIsConnected(true);
    // we just connected, need to clear the buffer (which was possibly overfilled)
    buffer.clear();
    // lastReceivedRTCM_ms = millis() - 100000;
    lastSentRTCM_ms = millis(); // We also need to reset the last sent time
    Trace("OK: Connected and authenticated to Caster");
    onConnected();
}
void NTRIPManager::Caster::_onConnectionFailed()
{
    failedCasterConnectionAttempts++;
    // by this time client.close() has been closed, if that was needed.

    // displayCasterIsConnected(false);
    failedCasterConnectionAttempts++;

    // we want to try re-connecting again in 10 seconds, but if that fails - wait for 5 minutes and repeat at 5 minutes 4 times.
    // also, total attempts over 1500 per day will get our IP address banned at the Caster. Go to doghouse if over 100 attempts.

    if (failedCasterConnectionAttempts < 2)
    {
        delay(10 * 1000); // first try reconnecting in 10 seconds
    }
    else
    {
        // progressively increase interval between connection attempts:
        delay(failedCasterConnectionAttempts * 5 * 60 * 1000);
    }

    if (failedCasterConnectionAttempts > maxFailedCasterConnectionAttempts)
    {
        Error("Exceeded max connection attempts. Waiting...");
        int minutesLeft = waitAfterFailedConnectionMinutes;
        while (minutesLeft > 0)
        {
            // displayWaitAfterFailedConnection(minutesLeft);
            delay(60 * 1000); // wait one minute
            minutesLeft--;
        }
        // now we are free to go:
        failedCasterConnectionAttempts = 0;
    }
    connectionState = ConnectionStates::DISCONNECTED;
    onConnectionFailed();
}

void NTRIPManager::Caster::onClosed()
{
}
void NTRIPManager::Caster::onConnected()
{
}
void NTRIPManager::Caster::onConnectionFailed()
{
}