// https://randomnerdtutorials.com/esp32-send-email-smtp-server-arduino-ide/

/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-send-email-smtp-server-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

// The Email Sending data object contains config and data to send
SMTPData smtpData;

#define emailSubject          "ESP32 RTK GPS Station Report"

// Callback function to get the Email sending status
void emailSendCallback(SendStatus info);

void sendEmail()
{
  if(isWifiConnected())
  {
    // Set the SMTP Server Email host, port, account and password
    smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
  
    // For library version 1.2.0 and later which STARTTLS protocol was supported, the STARTTLS will be 
    // enabled automatically when port 587 was used, or enable it manually using setSTARTTLS function.
    //smtpData.setSTARTTLS(true);
  
    // Set the sender name and Email
    smtpData.setSender("ESP32", emailSenderAccount);
  
    // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
    smtpData.setPriority("Normal");
    //smtpData.setPriority("High");
  
    // Set the subject
    smtpData.setSubject(emailSubject);
  
    // Set the message with HTML format
    smtpData.setMessage("<div style=\"color:#2f4468;\"><h2>Your RTK Base Station</h2><p>- Sent from ESP32 board</p></div>", true);
    // Set the email message in text format (raw)
    //smtpData.setMessage("Your RTK Base Station", false);
  
    // Add recipients, you can add more than one recipient
    smtpData.addRecipient(emailRecipient);
    //smtpData.addRecipient("YOUR_OTHER_RECIPIENT_EMAIL_ADDRESS@EXAMPLE.com");
  
    smtpData.setSendCallback(emailSendCallback);
  
    // Start sending Email, can be set callback function to track the status
    if (!MailClient.sendMail(smtpData))
    {
      //Error("Error sending Email, " + MailClient.smtpErrorReason());
    }
  
    // Clear all data from Email object to free memory
    smtpData.empty();
  }
}

// Callback function to get the Email sending status
void emailSendCallback(SendStatus msg)
{
  // Print the current status
  //Trace(msg.info());

  // Do something when complete
  if (msg.success()) {
    //Trace("OK: email sent successfully");
  }
}
