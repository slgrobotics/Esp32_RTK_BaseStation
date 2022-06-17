// // https://randomnerdtutorials.com/esp32-send-email-smtp-server-arduino-ide/

// /*
//   Rui Santos
//   Complete project details at https://RandomNerdTutorials.com/esp32-send-email-smtp-server-arduino-ide/

//   Permission is hereby granted, free of charge, to any person obtaining a copy
//   of this software and associated documentation files.

//   The above copyright notice and this permission notice shall be included in all
//   copies or substantial portions of the Software.
// */
// #include "EmailSender.h"

// // The Email Sending data object contains config and data to send
// // SMTPData smtpData;
// SMTPSession smtp;
// /* Callback function to get the Email sending status */
// void smtpCallback(SMTP_Status status);

// #define emailSubject "ESP32 RTK GPS Station Report"

// void sendEmail()
// {
//   smtp.debug(1);

//   /* Set the callback function to get the sending results */
//   smtp.callback(smtpCallback);

//   /* Declare the session config data */
//   ESP_Mail_Session session;

//   /* Set the session config */
//   session.server.host_name = SMTP_HOST;
//   session.server.port = SMTP_PORT;
//   session.login.email = EMAIL_SENDER_ACCOUNT;
//   session.login.password = EMAIL_SENDER_PASSWORD;
//   session.login.user_domain = EMAIL_SENDER_DOMAIN;

//   /* Set the NTP config time */
//   session.time.ntp_server = F("pool.ntp.org,time.nist.gov");
//   session.time.gmt_offset = 3;
//   session.time.day_light_offset = 0;

//   /* Declare the message class */
//   SMTP_Message message;

//   /* Set the message headers */
//   message.sender.name = EMAIL_SENDER_NAME;     // This will be used with 'MAIL FROM' command and 'From' header field.
//   message.sender.email = EMAIL_SENDER_ACCOUNT; // This witll be used with 'From' header field.
//   message.subject = EMAIL_SUBJECT;
//   message.addRecipient(EMAIL_RECIPIENT_NAME, EMAIL_RECIPIENT_ADDRESS); // This will be used with RCPT TO command and 'To' header field.

//   String msg = "<div style=\"color:#2f4468;\"><h2>Your RTK Base Station</h2><p>- Sent from ESP32 board</p></div>";
//   message.html.content = msg;
//   message.html.charSet = F("us-ascii");
//   message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
//   message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

//   // message.addHeader(F("Message-ID: <abcde.fghij@gmail.com>"));

//   // For Root CA certificate verification (ESP8266 and ESP32 only)
//   // session.certificate.cert_data = rootCACert;
//   // or
//   // session.certificate.cert_file = "/path/to/der/file";
//   // session.certificate.cert_file_storage_type = esp_mail_file_storage_type_flash; // esp_mail_file_storage_type_sd
//   // session.certificate.verify = true;

//   // The WiFiNINA firmware the Root CA certification can be added via the option in Firmware update tool in Arduino IDE

//   /* Connect to server with the session config */

//   // Library will be trying to sync the time with NTP server if time is never sync or set.
//   // This is 10 seconds blocking process.
//   // If time synching was timed out, the error "NTP server time synching timed out" will show via debug and callback function.
//   // You can manually sync time by yourself with NTP library or calling configTime in ESP32 and ESP8266.
//   // Time can be set manually with provided timestamp to function smtp.setSystemTime.

//   //
//   if (!smtp.connect(&session))
//     return;

//   /* Start sending Email and close the session */
//   if (!MailClient.sendMail(&smtp, &message))
//     Serial.println("Error sending Email, " + smtp.errorReason());

//   // to clear sending result log
//   // smtp.sendingResult.clear();

//   ESP_MAIL_PRINTF("Free Heap: %d\n", MailClient.getFreeHeap());
// }

// /* Callback function to get the Email sending status */
// void smtpCallback(SMTP_Status status)
// {
//   /* Print the current status */
//   Serial.println(status.info());

//   /* Print the sending result */
//   if (status.success())
//   {
//     // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
//     // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
//     // In ESP32 and ESP32, you can use Serial.printf directly.

//     Serial.println("----------------");
//     ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
//     ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
//     Serial.println("----------------\n");
//     struct tm dt;

//     for (size_t i = 0; i < smtp.sendingResult.size(); i++)
//     {
//       /* Get the result item */
//       SMTP_Result result = smtp.sendingResult.getItem(i);

//       // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
//       // your device time was synched with NTP server.
//       // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
//       // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)
//       time_t ts = (time_t)result.timestamp;
//       localtime_r(&ts, &dt);

//       ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
//       ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
//       ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
//       ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
//       ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
//     }
//     Serial.println("----------------\n");

//     // You need to clear sending result as the memory usage will grow up.
//     smtp.sendingResult.clear();
//   }
// }