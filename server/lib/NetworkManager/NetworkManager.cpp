#include "NetworkManager.h"

WiFiClient client;

IPAddress NetworkManager::getIP()
{
    return WiFi.localIP();
}
bool NetworkManager::_connect()
{
    if (isConnected())
    {
        return true;
    }
    // displayStateConnectingToWifi();
    WiFi.begin(_ssid, _password);

    int tries = 0;

    while (!isConnected() && ++tries <= 10)
    {
        // displayWifiConnecting(tries);
        delay(2000);
    }

    if (isConnected())
    {
        char traceBuffer[512];

        snprintf(traceBuffer, sizeof(traceBuffer), "\nOK: WiFi connected, local IP:");
        Serial.print(traceBuffer);
        Serial.println(WiFi.localIP());
        return true;
    }

    Error("Failed to connect to WiFi");
    return false;
}

void NetworkManager::work()
{

    if (isConnected() || _connect())
    {
        // displayWifiConnected();
    }
}

// NetworkManager::onConnected(IPAddress ipAddress)
// {
//     //
// }
// NetworkManager::onDisconnected()
// {
//     //
// }