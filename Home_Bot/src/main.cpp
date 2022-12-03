#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> 
#include "CONFIGS.hpp"
#include <mutex>

#define LED                     2
#define BOT_DELAY               500        // Milliseconds between updates
#define SEALEVELPRESSURE_HPA    1014.0F    // Sea level pressure in hPa
#define TEMPERATURE_OFFSET      -2.0F      // offset to compensate the temperature sensor

using namespace std;

// Create telegram bot object
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Create a mutex
SemaphoreHandle_t mtx;

// Create global variables
bool ledState;


/**
 * @brief Blink LED
 * @param nbBlink Number of blink (default 10)
 * @param delayBlink Delay between blink (default 100ms)
 */
void blinkLED(byte nbBlink = 10, byte delayTime = 100)
{
    for (byte i = 0; i < nbBlink; i++)
    {
        digitalWrite(LED, HIGH);
        delay(delayTime);
        digitalWrite(LED, LOW);
        delay(delayTime);
    }
}


/**
 * @brief Return welcome message
 * @param name Name to welcome
 * @param help Help message
 * @return Welcome message
 */
String returnWelcomeMessage(String name, bool help = false)
{
    String welcome = "";
    if (help == false)
    {
        welcome += "Welcome, " + name + "!\n";
    }
    welcome += "Use the following commands to control your bot :\n\n";
    welcome += "/led_on to turn GPIO ON \n";
    welcome += "/led_off to turn GPIO OFF \n";
    welcome += "/state to request current GPIO state \n";
    welcome += "/blink to blink LED \n";
    welcome += "/help to display this message \n";
    welcome += "/coreID to display which core is used by this bot \n";
    return welcome;
}


/**
 * @brief Randle what happens when you receive new messages
 * 
 * @param numNewMessages Number of new messages received
 */
void handleNewMessages(int numNewMessages)
{
    for (int i = 0; i < numNewMessages; i++)
    {
        // Chat id of the requester
        String chatID = String(bot.messages[i].chat_id);
        if ((chatID != CHAT_ID_1) && (chatID != CHAT_ID_2) && (chatID != CHAT_ID_GROUP))
        {
            bot.sendMessage(chatID, "You are not authorized to use this bot.");
            continue;
        }

        String text     = bot.messages[i].text;
        String fromName = bot.messages[i].from_name;

        if (text == "/start")
        {
            bot.sendMessage(chatID, returnWelcomeMessage(fromName));
        }

        else if (text == "/led_on")
        {
            bot.sendMessage(chatID, "LED state set to ON");
            ledState = HIGH;
            digitalWrite(LED, ledState);
        }

        else if (text == "/led_off")
        {
            bot.sendMessage(chatID, "LED state set to OFF");
            ledState = LOW;
            digitalWrite(LED, ledState);
        }

        else if (text == "/state")
        {
            if (ledState == true)
            {
                bot.sendMessage(chatID, "LED is ON");
            }
            else
            {
                bot.sendMessage(chatID, "LED is OFF");
            }
        }

        else if (text == "/blink")
        {
            bot.sendMessage(chatID, "LED will blink");
            blinkLED();
        }

        else if (text == "/help")
        {
            bot.sendMessage(chatID, returnWelcomeMessage(fromName, true));
        }

        else if (text == "/coreID")
        {
            bot.sendMessage(chatID, "This bot is running on core " + String(xPortGetCoreID()));
        }

        else
        {
            bot.sendMessage(chatID, "Invalid command");
        }
    }
}


/**
 * @brief Task executed by Bot to send and receive messages to Telegram
 * @param pvParameters Task parameters
 */
void botTask(void *pvParameters)
{
    while (1)
    {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while (numNewMessages)
        {
            if (xSemaphoreTake(mtx, portMAX_DELAY))
            {
                Serial.println("Bot task, got a message from Telegram.");
                xSemaphoreGive(mtx);
            }
            handleNewMessages(numNewMessages);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }

        // Wait for BOT_DELAY
        vTaskDelay(BOT_DELAY / portTICK_PERIOD_MS);
    }
}


void setup() 
{
    byte attempts = 0;
    ledState      = false;

    // Initialize serial
    Serial.begin(115200);
    Serial.println("Starting...");

    // Create a mutex
    mtx = xSemaphoreCreateMutex();

    // Initialize LED
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
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
    
    // Start bot task
    xTaskCreatePinnedToCore(botTask, "botTask", 10000, NULL, 1, NULL, 0);
}

void loop() 
{
}