
#ifndef ELYIR_SECRETS_H
#define ELYIR_SECRETS_H
// Your WiFi credentials
const char *ssid = "Wifi_Name";
const char *password = "Wifi_Password";

// RTK2Go works well and is free
//  const char casterHost[] = "rtk2go.com";
//  const uint16_t casterPort = 2101;
//  const char mountPoint[] = "bldr_dwntwn2"; //The mount point you want to push data to
//  const char mountPointPW[] = "WR5wRo4H";

// Emlid Caster also works well and is free
const char casterHost[] = "caster.emlid.com";
const uint16_t casterPort = 2101;
const char mountPoint[] = "-----"; // The mount point you want to push data to
const char mountPointPW[] = "------";

const char *ntrip_server_name = "Elyir_NTRIP_HOLTS_SUMMIT";

// To send Email using Gmail use port 465 (SSL) and SMTP Server smtp.gmail.com
// see https://randomnerdtutorials.com/esp32-send-email-smtp-server-arduino-ide/ - how to set up Google account

// YOU MUST ENABLE less secure app option https://myaccount.google.com/lesssecureapps?pli=1

#define EMAIL_SENDER_NAME "Elyir NTRIP Caster Admin"
#define EMAIL_SENDER_ACCOUNT "sender-gmail@gmail.com"
#define EMAIL_SENDER_PASSWORD "----"
#define EMAIL_SENDER_DOMAIN "elyir.com"
// #define EMAIL_RECIPIENT "me@hotmail.com"
#define EMAIL_RECIPIENT_NAME "Elyir Admin"
#define EMAIL_RECIPIENT_ADDRESS "recipient-email@gmail.com"
#define EMAIL_SUBJECT "ESP32 RTK GPS Station Report"
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#endif