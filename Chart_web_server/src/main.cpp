#include <Arduino.h>
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "DHT.h"
#include "CONFIGS.hpp"
#include "NTPClient.h"
#include "WiFiUdp.h"

#define DHTPIN   14
#define DHTTYPE  DHT11
#define LED      2
#define MAX_DATA 10

// Create DHT object
DHT dht(DHTPIN, DHTTYPE);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Create struct to store sensor data
typedef struct 
{
    unsigned long time;
    float temperature;
    float humidity;
} Data;
Data data[MAX_DATA];
static unsigned char data_size; // counter to store data

/**
 * @brief Read temperature from DHT11 sensor
 */
float readDHTTemperature()
{
    float t = dht.readTemperature();
    static float t_mem;
    if (isnan(t))
    {
        Serial.println("Failed to read temperature from DHT sensor ! Kept the old value");
        return t_mem;
    }
    t_mem = t;
    Serial.println("Read temperature : " + String(t) + "°C");
    return t;
}

/**
 * @brief Read humidity from DHT11 sensor
 */
float readDHTHumidity()
{
    float h = dht.readHumidity();
    static float h_mem;
    if (isnan(h))
    {
        Serial.println("Failed to read humidity from DHT sensor ! Kept the old value");
        return h_mem;
    }
    h_mem = h;
    Serial.println("Read humidity : " + String(h) + "°C");
    return h;
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
 * @brief Read temperature and humidity from DHT11 sensor
 */
void addDataToStruct()
{
    if (data_size == MAX_DATA)
    {
        memmove(data, data + 1, sizeof(data) - sizeof(data[0]));
        data[MAX_DATA - 1].time        = readTime();
        data[MAX_DATA - 1].temperature = readDHTTemperature();
        data[MAX_DATA - 1].humidity    = readDHTHumidity();
    }
    else
    {
        data[data_size].time        = readTime();
        data[data_size].temperature = readDHTTemperature();
        data[data_size].humidity    = readDHTHumidity();
        data_size++;
    }
}

/**
 * @brief Convert the structure to a string where each data is separated by a space
 */
String convertStructToString()
{
    String str = "";
    for (int i = 0; i < data_size; i++)
    {
        str += String(data[i].time) + " " + String(data[i].temperature) + " " + String(data[i].humidity) + "\n";
    }
    Serial.println("Convert struct to string : \n" + str);
    return str;
}

/**
 * @brief Blink LED 5 times when esp32 is ready
 */
inline void blinkWhenReady()
{
    for (byte i = 0; i < 5; i++)
    {
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
        delay(100);
    }
    digitalWrite(LED, HIGH);
}

void setup()
{
    // Initialize variables
    unsigned char attempts  = 0;
    bool led_on             = false;
    data_size               = 0;
    memset(&data[0], 0, sizeof(data));

    // Initialize serial 
    Serial.begin(115200);
    pinMode(LED, OUTPUT);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Initialize DHT sensor
    dht.begin();
    delay(1000);

    // Connect to Wi-Fi
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi.. Attempt " + String(++attempts));
        digitalWrite(LED, led_on);
        led_on = !led_on;
        if (attempts > 10)
        {
            Serial.println("Failed to connect to WiFi");
            ESP.restart();
        }
    }

    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());

    // Initialize NTP client
    timeClient.begin();
    timeClient.setTimeOffset(7200);
    timeClient.setUpdateInterval(2000);

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/index.html", String()); 
    });

    // Route for style.css file
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/style.css", "text/css"); 
    });

    // Route for js file
    server.on("/function.js", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/function.js", "text/javascript"); 
    });
    
    // Print chart in another page
    server.on("/chart", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(SPIFFS, "/chart.html", String());
    });

    // Read temperature and humidity
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send(200, "text/plain", convertStructToString().c_str());
    });

    // Start server
    server.begin();

    // Light the LED when connected
    blinkWhenReady();
}

void loop()
{
    addDataToStruct();
    delay(2000);
}