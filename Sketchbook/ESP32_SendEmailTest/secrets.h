
// REPLACE WITH YOUR NETWORK CREDENTIALS:

const char* ssid = "your_network_id_here";
const char* password =  "your_password_here";

// To send Email using Gmail use port 465 (SSL) and SMTP Server smtp.gmail.com
// see https://randomnerdtutorials.com/esp32-send-email-smtp-server-arduino-ide/ - how to set up Google account

// YOU MUST ENABLE less secure app option https://myaccount.google.com/lesssecureapps?pli=1

#define emailSenderAccount    "some_account@gmail.com"    
#define emailSenderPassword   "1234567890pwd"
#define emailRecipient        "me@hotmail.com"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
