#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> 
#include "CONFIGS.hpp"
#include <mutex>
#include <esp_now.h>
#include "Adafruit_BME680.h"
#include "NTPClient.h"
#include "WiFiUdp.h"

#define LED                     2
#define BOT_DELAY               500        // Milliseconds between updates of bot
#define SENSOR_DELAY            1000       // Milliseconds between updates of sensors
#define SEALEVELPRESSURE_HPA    1014.0F    // Sea level pressure in hPa
#define TEMPERATURE_OFFSET      -2.0F      // offset to compensate the temperature sensor

using namespace std;

// Create telegram bot object
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Create a mutex
SemaphoreHandle_t mtx;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Stores id of the rooms
enum ID 
{
    LIVING_ROOM
};

// Message to receive
typedef struct  
{
    uint8_t id;
    float temperature;
    float humidity;
    float pressure;
    float altitude;
    float gas_resistance;
    unsigned long time;
} Message_bme680;

// Create global variables
bool ledState;
Message_bme680 message;
Adafruit_BME680 bme;
const String rooms[] = {"Living room"};


/**
 * @brief Blink LED
 * @param nbBlink Number of blink (default 10)
 * @param delayBlink Delay between blink (default 100ms)
 */
void blinkLED(uint8_t nbBlink = 10, uint8_t delayTime = 100)
{
    for (uint8_t i = 0; i < nbBlink; i++)
    {
        digitalWrite(LED, HIGH);
        delay(delayTime);
        digitalWrite(LED, LOW);
        delay(delayTime);
    }
}


/**
 * @brief Read temperature from BME680 sensor
 * @return Temperature in °C
 */
float readBME680Temperature()
{
    float t = bme.readTemperature();
    static float t_mem;
    if (isnan(t))
    {
        Serial.println("Failed to read temperature from BME680 sensor ! Kept the old value");
        return t_mem + TEMPERATURE_OFFSET;
    }
    t_mem = t;
    return t + TEMPERATURE_OFFSET;
}


/**
 * @brief Read humidity from BME680 sensor
 * @return Humidity in %
 */
float readBME680Humidity()
{
    float h = bme.readHumidity();
    static float h_mem;
    if (isnan(h))
    {
        Serial.println("Failed to read humidity from BME680 sensor ! Kept the old value");
        return h_mem;
    }
    h_mem = h;
    return h;
}


/**
 * @brief Read pressure from BME680 sensor
 * @return Pressure in hPa
 */
float readBME680Pressure()
{
    float p = bme.readPressure() / 100.0F;
    static float p_mem;
    if (isnan(p))
    {
        Serial.println("Failed to read pressure from BME680 sensor ! Kept the old value");
        return p_mem;
    }
    p_mem = p;
    return p;
}


/**
 * @brief Read altitude from BME680 sensor
 * @return Altitude in meters
 */
float readBME680Altitude()
{
    float a = bme.readAltitude(SEALEVELPRESSURE_HPA);
    static float a_mem;
    if (isnan(a))
    {
        Serial.println("Failed to read altitude from BME680 sensor ! Kept the old value");
        return a_mem;
    }
    a_mem = a;
    return a;
}


/**
 * @brief Read gas resistance from BME680 sensor
 * @return Gas resistance in kOhm
 */
float readBME680GasResistance()
{
    float r = bme.readGas() / 1000.0F;
    static float r_mem;
    if (isnan(r))
    {
        Serial.println("Failed to read gas resistance from BME680 sensor ! Kept the old value");
        return r_mem;
    }
    r_mem = r;
    return r;
}


/**
 * @brief Read epoch from NTP server in milliseconds
 */
unsigned long readTime()
{
    timeClient.update();
    return timeClient.getEpochTime();
}


/**
 * @brief Read all data from BME280 sensor
 */
void updateData()
{
    message.temperature     = readBME680Temperature();
    message.humidity        = readBME680Humidity();
    message.pressure        = readBME680Pressure();
    message.altitude        = readBME680Altitude();
    message.gas_resistance  = readBME680GasResistance();
    message.time            = readTime();
}


/**
 * @brief Convert struct to string
 * @param message Struct to convert
 * @return String
 */
String structToString(Message_bme680 message)
{
    String str = "";
    str += "Time: " + String(message.time) + " s\n";
    str += "ID: " + rooms[message.id] + "\n";
    str += "Temperature: " + String(message.temperature) + " °C\n";
    str += "Humidity: " + String(message.humidity) + " %\n";
    str += "Pressure: " + String(message.pressure) + " hPa\n";
    str += "Altitude: " + String(message.altitude) + " m\n";
    str += "Gas resistance: " + String(message.gas_resistance) + " kOhm\n";
    return str;
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
    welcome += "/read_sensor to display sensor data \n";
    return welcome;
}


/**
 * @brief Randle what happens when you receive new messages
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

        else if (text == "/read_sensor")
        {
            bot.sendMessage(chatID, structToString(message));
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


/**
 * @brief Task executed by sensor to read data from BME680 sensor
 * @param pvParameters Task parameters
 */
void sensorTask(void *pvParameters)
{
    while (1)
    {
        // Read data from BME680 sensor
        if (xSemaphoreTake(mtx, portMAX_DELAY))
        {
            Serial.println("Sensor task, reading data from BME680 sensor.");
            updateData();
            xSemaphoreGive(mtx);
        }

        // Wait for SENSOR_DELAY
        vTaskDelay(SENSOR_DELAY / portTICK_PERIOD_MS);
    }
}


/**
 * @brief Setup function
 */
void setup() 
{
    uint8_t attempts = 0;
    ledState         = false;
    memset(&message, 0, sizeof(message));

    // Initialize serial
    Serial.begin(115200);
    Serial.println("Starting...");

    // Create a mutex
    mtx = xSemaphoreCreateMutex();

    // Initialize LED
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    // Initialize BME680
    if (!bme.begin())
    {
        Serial.println("Could not find a valid BME680 sensor, check wiring!");
        ESP.restart();
    }

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

    // Initialize NTP client
    timeClient.begin();
    timeClient.setTimeOffset(3600);
    timeClient.setUpdateInterval(1000);
    
    // Start both tasks
    xTaskCreatePinnedToCore(botTask, "botTask", 10000, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(sensorTask, "sensorTask", 10000, NULL, 1, NULL, 1);
}

/**
 * @brief Loop function
 */
void loop() 
{
}