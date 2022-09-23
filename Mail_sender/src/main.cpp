#include <Arduino.h>
#include "WiFi.h"
#include "CONFIGS.hpp"
#include "ESP_Mail_Client.h"

#define LED 2

// Define smtp object
SMTPSession      smtp;
SMTP_Message     message;
ESP_Mail_Session session;

void setup()
{
    Serial.begin(115200);
    pinMode(LED, OUTPUT);

    // Connect to Wi-Fi
    unsigned attempts   = 0;
    bool     LED_on     = false;
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi.. Attempt " + String(++attempts));
        digitalWrite(LED, LED_on);
        LED_on = !LED_on;

        if (attempts > 10)
        {
            Serial.println("Failed to connect to WiFi, rebooting..");
            ESP.restart();
        }
    }

    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());

    // Light the LED when setup is done
    digitalWrite(LED, HIGH);
    
    // set smtp server and port
    smtp.debug(1);
    session.server.host_name  = SMTP_SERVER;
    session.server.port       = SMTP_PORT;
    session.login.email       = SMTP_USER;
    session.login.password    = SMTP_PASS;
    session.login.user_domain = "";

    if (smtp.connect(&session))
    {
        Serial.println("SMTP server connected.");
    }
    else
    {
        Serial.println("SMTP server connection failed.");
        return;
    }

    // set message parameters
    message.sender.name    = "ESP32";
    message.sender.email   = SMTP_USER;
    message.subject        = "ESP32 Mail Client Test";
    message.addRecipient("Test", SMTP_DEST);

    // set message content
    String content       = "Hello, this is a test message from ESP32 Mail Client.";
    message.text.content = content.c_str();
    message.text.charSet = "ascii";

    // send message
    if (MailClient.sendMail(&smtp, &message))
    {
        Serial.println("Message sent successfully");
    }
    else
    {
        Serial.println("Message sending failed : " + smtp.errorReason());
    }
}

void loop()
{
}