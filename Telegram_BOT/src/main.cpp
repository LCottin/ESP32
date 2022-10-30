#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> 
#include <ArduinoJson.h>
#include "CONFIGS.hpp"

#define LED 2

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

bool ledState = LOW;

void blinkLED()
{
    for (int i = 0; i < 50; i++)
    {
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
        delay(100);
    }
}

String returnWelcomeMessage(String name)
{
    String welcome = "Welcome, " + name + ".\n";
    welcome += "Use the following commands to control your outputs.\n\n";
    welcome += "/led_on to turn GPIO ON \n";
    welcome += "/led_off to turn GPIO OFF \n";
    welcome += "/state to request current GPIO state \n";
    welcome += "/blink to blink LED \n";
    return welcome;
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages)
{
    Serial.println("handleNewMessages");
    Serial.println(String(numNewMessages));

    for (int i = 0; i < numNewMessages; i++)
    {
        // Chat id of the requester
        String chatID = String(bot.messages[i].chat_id);
        if ((chatID != CHAT_ID_1) && (chatID != CHAT_ID_2))
        {
            bot.sendMessage(chatID, "You are not authorized to use this bot.");
            continue;
        }

        // Print the received message
        String text = bot.messages[i].text;
        Serial.println(text);

        String fromName = bot.messages[i].from_name;

        if (text == "/start")
        {
            
            bot.sendMessage(chatID, returnWelcomeMessage(fromName), "");
        }

        if (text == "/led_on")
        {
            bot.sendMessage(chatID, "LED state set to ON", "");
            ledState = HIGH;
            digitalWrite(LED, ledState);
        }

        if (text == "/led_off")
        {
            bot.sendMessage(chatID, "LED state set to OFF", "");
            ledState = LOW;
            digitalWrite(LED, ledState);
        }

        if (text == "/state")
        {
            if (digitalRead(LED))
            {
                bot.sendMessage(chatID, "LED is ON", "");
            }
            else
            {
                bot.sendMessage(chatID, "LED is OFF", "");
            }
        }

        if (text == "/blink")
        {
            bot.sendMessage(chatID, "LED will blink", "");
            blinkLED();
        }
    }
}

void setup()
{
    byte attempts = 0;
    Serial.begin(115200);

    pinMode(LED, OUTPUT);
    digitalWrite(LED, ledState);

    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi.. Attempt " + String(++attempts));
        digitalWrite(LED, ledState);
        ledState = !ledState;
        if (attempts > 10)
        {
            Serial.println("Failed to connect to WiFi. Restarting... \n\n");
            ESP.restart();
        }
    }
    Serial.println(WiFi.localIP());
}

void loop()
{
    unsigned long lastTimeBotRan;
    if (millis() > lastTimeBotRan + 1000)
    {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        Serial.println("numNewMessages = " + String(numNewMessages));
        while (numNewMessages)
        {
            Serial.println("got response");
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }
}