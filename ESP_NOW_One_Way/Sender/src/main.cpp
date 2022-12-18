#include <Arduino.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include "CONFIGS.hpp"
#include "Adafruit_BME280.h"
#include "NTPClient.h"
#include "WiFiUdp.h"

#define LED                  2
#define SEALEVELPRESSURE_HPA 1014.0F    // Sea level pressure in hPa
#define TEMPERATURE_OFFSET   -2.0F      // offset to compensate the temperature sensor
#define MY_ID                BEDROOM    // ID of the room

// Mac address of the receiver and sender
constexpr uint8_t receiverAddress[] = {0xC8, 0xF0, 0x9E, 0xA3, 0x52, 0xA8};

// Stores id of the rooms
enum ID 
{
    BEDROOM, 
    LIVING_ROOM
};

// Message to send or receive
typedef struct  
{
    uint8_t id;
    float temperature;
    float humidity;
    float pressure;
    float altitude;
    unsigned long time;
} Message;
Message message;

// Create BME280 object
Adafruit_BME280 bme;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Create struct for peer info
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
    Serial.print("Last Packet Send Status :\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

float readTemperature()
{
    float t = bme.readTemperature();
    static float t_mem;
    if (isnan(t))
    {
        Serial.println("Failed to read temperature from BME280 sensor ! Kept the old value");
        return t_mem + TEMPERATURE_OFFSET;
    }
    t_mem = t;
    Serial.println("Read temperature : " + String(t + TEMPERATURE_OFFSET) + "°C");
    return t + TEMPERATURE_OFFSET;
}

float readHumidity()
{
    float h = bme.readHumidity();
    static float h_mem;
    if (isnan(h))
    {
        Serial.println("Failed to read humidity from BME280 sensor ! Kept the old value");
        return h_mem;
    }
    h_mem = h;
    Serial.println("Read humidity : " + String(h) + "°C");
    return h;
}

float readPressure()
{
    float p = bme.readPressure() / 100.0F;
    static float p_mem;
    if (isnan(p))
    {
        Serial.println("Failed to read pressure from BME280 sensor ! Kept the old value");
        return p_mem;
    }
    p_mem = p;
    Serial.println("Read pressure : " + String(p) + "hPa");
    return p;
}

float readAltitude()
{
    float a = bme.readAltitude(SEALEVELPRESSURE_HPA);
    static float a_mem;
    if (isnan(a))
    {
        Serial.println("Failed to read altitude from BME280 sensor ! Kept the old value");
        return a_mem;
    }
    a_mem = a;
    Serial.println("Read altitude : " + String(a) + "m");
    return a;
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
 * @brief Read temperature and humidity from BME280 sensor
 */
void updateData()
{
    // Read temperature, humidity, pressure and altitude from BME280 sensor
    message.temperature = readTemperature();
    message.humidity    = readHumidity();
    message.pressure    = readPressure();
    message.altitude    = readAltitude();
    message.time        = readTime();

    Serial.println("Data updated");
}

/**
 * @brief Send data to receiver
 */ 
void sendData()
{
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(receiverAddress, (uint8_t *) &message, sizeof(message));
    if (result == ESP_OK)
    {
        Serial.println("Data sent successfully");
    }
    else
    {
        Serial.println("Error sending the data : " + String(result));
    }
}
 
void setup() 
{
    // Initialize variables
    unsigned char attempts  = 0;
    bool led_on             = false;
    message.id              = MY_ID;

    // Init Serial Monitor
    Serial.begin(115200);

    // Initialize LED
    pinMode(LED, OUTPUT);
    digitalWrite(LED, led_on);

    // Init bme280 sensor
    if (!bme.begin(0x76))
    {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        ESP.restart();
    }

    // Set device in STA mode to begin with
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(WIFI_SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi.. Attempt " + String(++attempts));
        digitalWrite(LED, led_on);
        led_on = !led_on;
        if (attempts > 10)
        {
            Serial.println("Failed to connect to WiFi");
            delay(2000);
            ESP.restart();
        }
    }
    led_on = false;
    digitalWrite(LED, led_on);
    Serial.println(WiFi.localIP());

    // Initialize NTP client
    timeClient.begin();
    timeClient.setUpdateInterval(1000);
    
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        delay(2000);
        ESP.restart();
    }

    // Register peer
    memcpy(peerInfo.peer_addr, receiverAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("Failed to add peer");
        delay(2000);
        ESP.restart();
    }

    esp_now_register_send_cb(OnDataSent);

    Serial.println(F("Sender ready"));
}
 
void loop() 
{
    // Update data
    updateData();

    // Send data
    sendData();

    // Wait 1 seconds
    delay(1000);
}