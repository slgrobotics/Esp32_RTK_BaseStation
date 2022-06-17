#ifndef ELYIR_NETWORK_MGR_H
#define ELYIR_NETWORK_MGR_H

#include "Tracing.h"
#include <WiFi.h>

class NetworkManager
{
private:
    bool _connect();
    const char *_ssid;
    const char *_password;

public:
    void setup(const char *ssid, const char *password)
    {
        _ssid = ssid;
        _password = password;
    }
    void work();
    void disconnect();
    bool isConnected()
    {
        return WiFi.status() == WL_CONNECTED;
    }
    IPAddress getIP();
    void onConnected(IPAddress ipAddress) __attribute__((weak));
    void onDisconnected() __attribute__((weak));
};
#endif