#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> 
#include "CONFIGS.hpp"
#include <mutex>
#include "Adafruit_BME680.h"
#include "NTPClient.h"
#include "WiFiUdp.h"
#include "esp_now.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"

#define LED                     2
#define BOT_DELAY               800        // Milliseconds between updates of bot
#define SENSOR_DELAY            1000       // Milliseconds between updates of sensors
#define ESP_NOW_DELAY           1000       // Milliseconds between updates of ESP-NOW
#define SEALEVELPRESSURE_HPA    1014.0F    // Sea level pressure in hPa
#define TEMPERATURE_OFFSET      -2.0F      // offset to compensate the temperature sensor
#define MAX_DATA                10         // Max number of data to store

using namespace std;

// Create telegram bot object
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Create a mutex
SemaphoreHandle_t mtx;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Create BME680 object
Adafruit_BME680 bme;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Stores id of the rooms
enum ID 
{
    LIVING_ROOM,
    BEDROOM,
    BATHROOM
};

// BME680 data
typedef struct  
{
    uint8_t id;
    float temperature;
    float humidity;
    float pressure;
    float altitude;
    float gas_resistance;
    uint32_t time;
} Message_bme680;

// BME280 data
typedef struct  
{
    uint8_t id;
    float temperature;
    float humidity;
    float pressure;
    float altitude;
    uint32_t time;
} Message_bme280;

// Data for living room
typedef struct
{
    uint8_t count;
    Message_bme680 data[MAX_DATA];
} Data_living_room;

// Data for bathroom
typedef struct
{
    uint8_t count;
    Message_bme280 data[MAX_DATA];
} Data_bathroom;

// Data for bedroom
typedef struct
{
    uint8_t count;
    Message_bme280 data[MAX_DATA];
} Data_bedroom;

// ESP-NOW data
typedef struct  
{
    bool received;
    Message_bme280 bme280_tmp;
} Incoming_data;

// Create global variables
volatile bool             ledState;
volatile Incoming_data    incoming_data;
volatile Data_living_room data_living_room;
volatile Data_bathroom    data_bathroom;
volatile Data_bedroom     data_bedroom;
const String rooms[] = 
{
    "Living room", 
    "Bedroom", 
    "Bathroom"
};

/* =================================================================== */

/***********************************************************************
 * @brief Blink LED
 * @param nbBlink Number of blink (default 10)
 * @param delayBlink Delay between blink (default 100ms)
 ***********************************************************************/
inline void blinkLED(const uint8_t nbBlink = 10, const uint8_t delayTime = 100)
{
    for (uint8_t i = 0; i < nbBlink; i++)
    {
        digitalWrite(LED, HIGH);
        delay(delayTime);
        digitalWrite(LED, LOW);
        delay(delayTime);
    }
}


/***********************************************************************
 * @brief Read temperature from BME680 sensor
 * @return Temperature in °C
 ***********************************************************************/
float readBME680Temperature()
{
    float t = bme.readTemperature();
    static float t_mem;
    if(isnan(t))
    {
        Serial.println("Failed to read temperature from BME680 sensor ! Kept the old value");
        return t_mem + TEMPERATURE_OFFSET;
    }
    t_mem = t;
    return t + TEMPERATURE_OFFSET;
}


/***********************************************************************
 * @brief Read humidity from BME680 sensor
 * @return Humidity in %
 ***********************************************************************/
float readBME680Humidity()
{
    float h = bme.readHumidity();
    static float h_mem;
    if(isnan(h))
    {
        Serial.println("Failed to read humidity from BME680 sensor ! Kept the old value");
        return h_mem;
    }
    h_mem = h;
    return h;
}


/***********************************************************************
 * @brief Read pressure from BME680 sensor
 * @return Pressure in hPa
 ***********************************************************************/
float readBME680Pressure()
{
    float p = bme.readPressure() / 100.0F;
    static float p_mem;
    if(isnan(p))
    {
        Serial.println("Failed to read pressure from BME680 sensor ! Kept the old value");
        return p_mem;
    }
    p_mem = p;
    return p;
}


/***********************************************************************
 * @brief Read altitude from BME680 sensor
 * @return Altitude in meters
 ***********************************************************************/
float readBME680Altitude()
{
    float a = bme.readAltitude(SEALEVELPRESSURE_HPA);
    static float a_mem;
    if(isnan(a))
    {
        Serial.println("Failed to read altitude from BME680 sensor ! Kept the old value");
        return a_mem;
    }
    a_mem = a;
    return a;
}


/***********************************************************************
 * @brief Read gas resistance from BME680 sensor
 * @return Gas resistance in kOhm
 ***********************************************************************/
float readBME680GasResistance()
{
    float r = bme.readGas() / 1000.0F;
    static float r_mem;
    if(isnan(r))
    {
        Serial.println("Failed to read gas resistance from BME680 sensor ! Kept the old value");
        return r_mem;
    }
    r_mem = r;
    return r;
}


/***********************************************************************
 * @brief Read epoch from NTP server in seconds
 ***********************************************************************/
uint32_t readTime()
{
    timeClient.update();
    return timeClient.getEpochTime();
}


/***********************************************************************
 * @brief Read all data from BME280 sensor and store it in all_data
 ***********************************************************************/
void updateBME680Data()
{
    uint8_t idx;

    if (data_living_room.count == MAX_DATA)
    {
        // Shift data
        for (uint8_t i = 0; i < MAX_DATA - 1; i++)
        {
            memcpy((void *)&data_living_room.data[i], (void *)&data_living_room.data[i + 1], sizeof(data_living_room.data[0]));
        }
        idx = MAX_DATA - 1;
    }
    else if (data_living_room.count == 0)
    {
        idx = 0;
        data_living_room.count++;
    }
    else
    {
        idx = data_living_room.count - 1;
        data_living_room.count++;
    }
    
    data_living_room.data[idx].time           = readTime();
    data_living_room.data[idx].temperature    = readBME680Temperature();
    data_living_room.data[idx].humidity       = readBME680Humidity();
    data_living_room.data[idx].pressure       = readBME680Pressure();
    data_living_room.data[idx].altitude       = readBME680Altitude();
    data_living_room.data[idx].gas_resistance = readBME680GasResistance();
}


/***********************************************************************
 * @brief Convert struct to string
 * @param lastOnly True if only the value of the last data is needed, false if all data is needed (default false)
 * @return String with all data, separated by ',' for each label, ";" for each data and '\n' for each room
 ***********************************************************************/
String structToString(const bool lastOnly = false)
{
    String str = "";
    if(lastOnly == false)
    {
        // Convert living room data
        for (uint8_t i = 0; i < data_living_room.count; i++)
        {
            str += rooms[data_living_room.data[i].id]+ ",";
            str += String(data_living_room.data[i].time) + ",";
            str += String(data_living_room.data[i].temperature) + ",";
            str += String(data_living_room.data[i].humidity) + ",";
            str += String(data_living_room.data[i].pressure) + ",";
            str += String(data_living_room.data[i].altitude) + ",";
            str += String(data_living_room.data[i].gas_resistance) + ";";
        }
        str += "\n";

        // Convert bedroom data
        for (uint8_t i = 0; i < data_bedroom.count; i++)
        {
            str += rooms[data_bedroom.data[i].id]+ ",";
            str += String(data_bedroom.data[i].time) + ",";
            str += String(data_bedroom.data[i].temperature) + ",";
            str += String(data_bedroom.data[i].humidity) + ",";
            str += String(data_bedroom.data[i].pressure) + ",";
            str += String(data_bedroom.data[i].altitude) + ";";
        }
        str += "\n";

        // Convert bathroom data
        for (uint8_t i = 0; i < data_bathroom.count; i++)
        {
            str += rooms[data_bathroom.data[i].id]+ ",";
            str += String(data_bathroom.data[i].time) + ",";
            str += String(data_bathroom.data[i].temperature) + ",";
            str += String(data_bathroom.data[i].humidity) + ",";
            str += String(data_bathroom.data[i].pressure) + ",";
            str += String(data_bathroom.data[i].altitude) + ";";
        }
    }
    else 
    {
        const uint8_t lastIdx_living_room = (data_living_room.count == 0) ? 0 : data_living_room.count - 1;
        const uint8_t lastIdx_bedroom     = (data_bedroom.count == 0)     ? 0 : data_bedroom.count - 1;
        const uint8_t lastIdx_bathroom    = (data_bathroom.count == 0)    ? 0 : data_bathroom.count - 1;

        str += "In living room:\n";
        str += "\t\tTime: " + String(data_living_room.data[lastIdx_living_room].time) + ";";
        str += "\t\tTemperature: " + String(data_living_room.data[lastIdx_living_room].temperature) + ";";
        str += "\t\tHumidity: " + String(data_living_room.data[lastIdx_living_room].humidity) + ";";
        str += "\t\tPressure: " + String(data_living_room.data[lastIdx_living_room].pressure) + ";";
        str += "\t\tAltitude: " + String(data_living_room.data[lastIdx_living_room].altitude) + ";";
        str += "\t\tGas resistance: " + String(data_living_room.data[lastIdx_living_room].gas_resistance) + "\n";

        str += "In bedroom:\n";
        str += "\t\tTime: " + String(data_bedroom.data[lastIdx_bedroom].time) + ";";
        str += "\t\tTemperature: " + String(data_bedroom.data[lastIdx_bedroom].temperature) + ";";
        str += "\t\tHumidity: " + String(data_bedroom.data[lastIdx_bedroom].humidity) + ";";
        str += "\t\tPressure: " + String(data_bedroom.data[lastIdx_bedroom].pressure) + ";";
        str += "\t\tAltitude: " + String(data_bedroom.data[lastIdx_bedroom].altitude) + "\n";

        str += "In bathroom:\n";
        str += "\t\tTime: " + String(data_bathroom.data[lastIdx_bathroom].time) + ";";
        str += "\t\tTemperature: " + String(data_bathroom.data[lastIdx_bathroom].temperature) + ";";
        str += "\t\tHumidity: " + String(data_bathroom.data[lastIdx_bathroom].humidity) + ";";
        str += "\t\tPressure: " + String(data_bathroom.data[lastIdx_bathroom].pressure) + ";";
        str += "\t\tAltitude: " + String(data_bathroom.data[lastIdx_bathroom].altitude) + "\n";
    }
    return str;
}


/***********************************************************************
 * @brief Return welcome message
 * @param name Name to welcome
 * @param help Help message
 * @return Welcome message
 ***********************************************************************/
String returnWelcomeMessage(const String &name, bool help = false)
{
    String welcome = "";
    if(help == false)
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


/***********************************************************************
 * @brief Randle what happens when you receive new messages
 * @param numNewMessages Number of new messages received
 ***********************************************************************/
void handleNewMessages(const int32_t numNewMessages)
{
    for (int32_t i = 0; i < numNewMessages; i++)
    {
        // Chat id of the requester
        String chatID = String(bot.messages[i].chat_id);
        if((chatID != CHAT_ID_1) && (chatID != CHAT_ID_2) && (chatID != CHAT_ID_GROUP))
        {
            bot.sendMessage(chatID, "You are not authorized to use this bot.");
            continue;
        }

        String text     = bot.messages[i].text;
        String fromName = bot.messages[i].from_name;

        if(text == "/start")
        {
            bot.sendMessage(chatID, returnWelcomeMessage(fromName));
        }

        else if(text == "/led_on")
        {
            bot.sendMessage(chatID, "LED state set to ON");
            ledState = HIGH;
            digitalWrite(LED, ledState);
        }

        else if(text == "/led_off")
        {
            bot.sendMessage(chatID, "LED state set to OFF");
            ledState = LOW;
            digitalWrite(LED, ledState);
        }

        else if(text == "/state")
        {
            if(ledState == true)
            {
                bot.sendMessage(chatID, "LED is ON");
            }
            else
            {
                bot.sendMessage(chatID, "LED is OFF");
            }
        }

        else if(text == "/blink")
        {
            bot.sendMessage(chatID, "LED will blink");
            blinkLED();
        }

        else if(text == "/help")
        {
            bot.sendMessage(chatID, returnWelcomeMessage(fromName, true));
        }

        else if(text == "/coreID")
        {
            bot.sendMessage(chatID, "This bot is running on core " + String(xPortGetCoreID()));
        }

        else if(text == "/read_sensor")
        {
            bot.sendMessage(chatID, structToString(true));
        }

        else
        {
            bot.sendMessage(chatID, "Invalid command");
        }
    }
}


/***********************************************************************
 * @brief Task executed by Bot to send and receive messages to Telegram
 * @param pvParameters Task parameters
 ***********************************************************************/
void botTask(void *pvParameters)
{
    while(true)
    {
        int32_t numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        while(numNewMessages)
        {
            if(xSemaphoreTake(mtx, portMAX_DELAY))
            {
                Serial.println("Bot task, got a message from Telegram.");
                handleNewMessages(numNewMessages);
                xSemaphoreGive(mtx);
            }
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }

        // Wait for BOT_DELAY
        vTaskDelay(BOT_DELAY / portTICK_PERIOD_MS);
    }
}


/***********************************************************************
 * @brief Task executed by sensor to read data from BME680 sensor
 * @param pvParameters Task parameters
 ***********************************************************************/
void sensorTask(void *pvParameters)
{
    while(true)
    {
        // Read data from BME680 sensor
        if(xSemaphoreTake(mtx, portMAX_DELAY))
        {
            Serial.println("Sensor task, reading data from BME680 sensor.");
            updateBME680Data();
            xSemaphoreGive(mtx);
        }

        // Wait for SENSOR_DELAY
        vTaskDelay(SENSOR_DELAY / portTICK_PERIOD_MS);
    }
}


/***********************************************************************
 * @brief Callback function to handle esp now reception
 * @param mac_addr 
 * @param incomingData 
 * @param len 
 ***********************************************************************/
void receiveData(const uint8_t *mac_addr, const uint8_t *incomingData, int32_t len)
{
    memcpy((void *)&incoming_data.bme280_tmp, incomingData, sizeof(Message_bme280));
    incoming_data.received = true;
}


/***********************************************************************
 * @brief Task executed by esp now to send and receive data
 * @param pvParameters Task parameters
 ***********************************************************************/
void espNowTask(void *pvParameters)
{
    while(true)
    {
        if(incoming_data.received)
        {
            if(xSemaphoreTake(mtx, portMAX_DELAY))
            {
                /* Make sure IDs are the same on the emittor side */
                Serial.println("Esp now task, got data from esp now.");
                if (incoming_data.bme280_tmp.id == BEDROOM)
                {
                    memcpy((void *)&data_bedroom.data, (void *)&incoming_data.bme280_tmp, sizeof(Message_bme280));
                    data_bedroom.count++;
                }
                else if (incoming_data.bme280_tmp.id == BATHROOM)
                {
                    memcpy((void *)&data_bathroom.data, (void *)&incoming_data.bme280_tmp, sizeof(Message_bme280));
                    data_bathroom.count++;
                }
                else
                {
                    Serial.println("Esp now task, invalid id received.");
                }

                incoming_data.received = false;
                xSemaphoreGive(mtx);
            }
        }

        // Wait for ESP_NOW_DELAY
        vTaskDelay(ESP_NOW_DELAY / portTICK_PERIOD_MS);
    }
}


/***********************************************************************
 * @brief Setup function
 ***********************************************************************/
void setup() 
{
    uint8_t attempts = 0;
    ledState         = false;
    memset((void *)&incoming_data,    0, sizeof(incoming_data));
    memset((void *)&data_bedroom,     0, sizeof(data_bedroom));
    memset((void *)&data_bathroom,    0, sizeof(data_bathroom));
    memset((void *)&data_living_room, 0, sizeof(data_living_room));
    for (uint8_t i = 0; i < MAX_DATA; i++)
    {
        data_living_room.data[i].id = LIVING_ROOM;
    }

    // Initialize serial
    Serial.begin(115200);

    // Create a mutex
    mtx = xSemaphoreCreateMutex();

    // Initialize LED
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    // Initialize BME680
    if(!bme.begin())
    {
        Serial.println("Could not find a valid BME680 sensor, check wiring!");
        ESP.restart();
    }

    // Initialize SPIFFS
    if(!SPIFFS.begin())
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        ESP.restart();
    }

    // Connect to Wi-Fi
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(SSID, PASSWORD);
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi.. Attempt " + String(++attempts));
        digitalWrite(LED, ledState);
        ledState = !ledState;
        if(attempts > 10)
        {
            Serial.println("Failed to connect to WiFi. Restarting... \n\n");
            ESP.restart();
        }
    }
    ledState = false;
    digitalWrite(LED, ledState);
    Serial.println(WiFi.localIP());

    // Initialize NTP client
    timeClient.begin();
    timeClient.setUpdateInterval(1000);

    // Initialize ESPNOW
    if(esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        ESP.restart();
    }
    esp_now_register_recv_cb(receiveData);

    // Initialize web server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/index.html");
    });
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/style.css", "text/css");
    });
    server.on("/function.js", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/function.js", "text/javascript");
    });
    server.on("/all_data", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(200, "text/plain", structToString(false).c_str());
    });
    server.on("/last_data", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(200, "text/plain", structToString(true).c_str());
    });
    server.on("/living_room.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/living_room/living_room.html");
    });
    server.on("/living_room.js", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/living_room/living_room.js", "text/javascript");
    });
    server.on("/bedroom.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/bedroom/bedroom.html");
    });
    server.on("/bedroom.js", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/bedroom/bedroom.js", "text/javascript");
    });

    server.begin();
    
    // Start tasks
    Serial.println("Starting tasks...");
    xTaskCreatePinnedToCore(botTask,       "botTask", 8192, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(sensorTask, "sensorTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(espNowTask, "espNowTask", 4096, NULL, 2, NULL, 1);
}


/***********************************************************************
 * @brief Loop function
 ***********************************************************************/
void loop() 
{
}