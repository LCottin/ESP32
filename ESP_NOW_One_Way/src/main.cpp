#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "CONFIGS.hpp"

#define LED 2

#define RECEIVER // SENDER or RECEIVER

// Mac address of the receiver and sender
uint8_t senderAddress[]   = {0xC8, 0xF0, 0x9E, 0xA3, 0x52, 0xA8};
uint8_t receiverAddress[] = {0xC8, 0xF0, 0x9E, 0xA3, 0x53, 0xCC};

// Message to send or receive
typedef struct  
{
    float f;
    int i;
} Message;

#ifdef SENDER
    esp_now_peer_info_t peerInfo;

    // callback when data is sent
    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) 
    {
        Serial.print("Last Packet Send Status :\t");
        Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    }

#elif defined(RECEIVER)
    // callback when data is received
    void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) 
    {
        Message message;
        memcpy(&message, incomingData, sizeof(Message));
        Serial.print("Received from : ");
        for (size_t i = 0; i < 6; i++)
        {
            Serial.print(mac_addr[i], HEX);
            if (i < 5)
                Serial.print(":");
        }
        Serial.print("\tPI = ");
        Serial.print(message.f);
        Serial.print("\tInt = ");
        Serial.println(message.i);
    }
#endif
 
void setup() 
{
    // Initialize variables
    unsigned char attempts  = 0;
    bool led_on             = false;

    // Init Serial Monitor
    Serial.begin(115200);

    // Connect to Wi-Fi station
    WiFi.mode(WIFI_STA);

#ifdef SENDER
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
            delay(2000);
            ESP.restart();
        }
    }
    
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

#elif defined(RECEIVER)
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) 
    {
        Serial.println("Error initializing ESP-NOW");
        delay(2000);
        ESP.restart();
    }
    esp_now_register_recv_cb(OnDataRecv);
#endif

    // Set LED pin as output
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
}
 
void loop() 
{
#ifdef SENDER
    Message message;
    message.f = PI;
    message.i = random(0, 100);

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)&message, sizeof(Message));

    if (result == ESP_OK) 
    {
        Serial.println("Sent with success");
    }
    else 
    {
        Serial.println("Error sending the data");
    }
    delay(500);
#endif
}