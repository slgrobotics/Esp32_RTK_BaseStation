#ifndef ELYIR_NTRIP_MANAGER_H
#define ELYIR_NTRIP_MANAGER_H

#include <WiFi.h>
#include <CircularBuffer.h>
#define MSG_BUFFER_SIZE 2048

#include "Tracing.h"

namespace NTRIPManager
{
    enum ConnectionStates
    {
        ALREADY_CONNECTED,
        CONNECTING,
        CONNECTED_AND_AUTHENTICATED,
        CONNECTED_BUT_AUTHENTICATING,
        FAILED_TO_OPEN_SOCKET,
        DISCONNECTING,
        DISCONNECTED,
        FAILED_TO_GET_CASTER_RESPONSE,
        RESPONSE_WAS_NOT_200
    };
    enum SendStates
    {
        NO_DATA_IN_BUFFER,
        DATA_SENT_SUCCESSFULLY,
        CLIENT_DISCONNECTED,
        IDLE_BY_SWITCH
    };
    enum CloseStates
    {
        NOT_CONNECTED,
        CLOSED
    };

    class Caster
    {
    private:
        CircularBuffer<uint8_t, MSG_BUFFER_SIZE> buffer;
        // Have a FIFO to log connection attempts, successful or not. Logs time in ms.
        // When this queue is overfilled (after 512 attempts in 24 hours), we'll wait in doghouse.
        // Older than 24 hours records are cleared:
        CircularBuffer<unsigned long, 512> connectionAttempts;
        unsigned long lastSentRTCM_ms; // Time of last data pushed to socket
        unsigned long lastReport_ms;
        unsigned long lastReceivedRTCM_ms;
        unsigned long lastWentToBreak; // when went to break for no RTCM
        unsigned long lastRTCMWaitReport_ms;
        unsigned long cntErrsReceivedRTCM_ms; // absolute error counters, to see where the problem might be
        unsigned long cntErrsSentRTCM_ms;
        unsigned long serverBytesSent; // Just a running total, on ESP32 4 bytes, max 4,294,967,295 (4GB)
        char traceBuffer[512];
        const unsigned long waitTimeBeforeInitialConnection_ms = 10000; // Let's wait 10 seconds before trying to connect for the first time
        const unsigned long rtcmWaithInterval_ms = 5000;                // Let's send a wait report every 5 seconds
        const unsigned long maxTimeBeforeHangup_ms = 10000;             // If we fail to get a complete RTCM frame after 10s, then disconnect from caster
        const unsigned long breakInterval_ms = 1000 * 60;               // hope that no-RTCM condition will be resolved in about a minute
        const unsigned long reportInterval_ms = 1000 * 10;              // not too often (Every 10 Seconds), avoid scrolling important events
        // const unsigned long reportInterval_ms = 1000 * 60 * 10;         // not too often, avoid scrolling important events
        ConnectionStates connectionState;

        WiFiClient client;

        const int maxFailedCasterConnectionAttempts = 10; // we retry at progressive intervals several times, then wait long to avoid IP ban
        const int waitAfterFailedConnectionMinutes = 60;  // to wait really long after retries, to avoid being banned by the Caster

        unsigned int failedCasterConnectionAttempts; // for Caster fast-reconnecting timing
        uint16_t _casterPort;
        const char *_casterHost;
        const char *_ntrip_server_name;
        const char *_mntpnt;
        const char *_mntpnt_pw;
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
        ConnectionStates _connect();
        bool _noNewRTCMRecivedForAWhile()
        {
            return (millis() - lastReceivedRTCM_ms) > maxTimeBeforeHangup_ms;
        }
        bool _noRTCMSentForAWhile()
        {
            return (millis() - lastSentRTCM_ms) > maxTimeBeforeHangup_ms;
        }
        void _sendData();
        void _onConnected();
        void _onClosed();
        void _onConnectionFailed();

    public:
        ~Caster()
        {
            close();
        }
        bool isConnected() { return client.connected(); }
        bool isConnectedAndAuthenticated()
        {
            return connectionState == ConnectionStates::CONNECTED_AND_AUTHENTICATED;
        }
        bool isConnectedButAuthenticating()
        {
            return connectionState == ConnectionStates::CONNECTED_BUT_AUTHENTICATING;
        }
        void setup(const uint16_t casterPort,
                   const char *casterHost,
                   const char *ntrip_server_name,
                   const char *mntpnt,
                   const char *mntpnt_pw);

        // Close connection to Caster
        CloseStates close();

        /*
         * Trying to send all bytes from rtcm FIFO queue to Caster's stream
         *
         * Returns:
         *  0  - all good, no data in queue
         *  1  - had data, sent successfully
         *  10 - client disconnected
         *
         */
        void send(uint8_t data);

        // Continously send data from rtcm FIFO queue to Caster's stream
        // Must be called in loop()
        void work(bool networkIsConnected);

        bool purgeConnectionAttempts();

        void onConnected() __attribute__((weak));
        void onConnectionFailed() __attribute__((weak));
        void onClosed() __attribute__((weak));
    };
} // namespace ntrip manager
#endif