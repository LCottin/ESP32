#include <Arduino.h>
#include "SPIFFS.h"
#include "CONFIGS.hpp"
#include "ESP_Mail_Client.h"
#include "esp_camera.h"
#include "SPI.h"
#include "driver/rtc_io.h"
#include <WiFi.h>

#define LED 4

#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
  
#else
  #error "Camera model not selected"
#endif

// Define smtp object
SMTPSession      smtp;
SMTP_Message     message;
SMTP_Attachment  attachment;
ESP_Mail_Session session;

// Define camera config
static camera_config_t config;

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
}

camera_fb_t * capturePhoto() 
{
    Serial.println("Capturing photo...");

    camera_fb_t *fb = nullptr;
    do
    {
        fb = esp_camera_fb_get();
        if (!fb)
        {
            Serial.println("Camera capture failed, retrying...");
            delay(100);
        }
    } while (fb == nullptr);

    Serial.println("Captured image.");
    return fb;
}

void sendPhoto() 
{
    // Preparing email
    Serial.println("Sending email...");
    
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
        esp_restart();
    }

    // Attach photo to email
    camera_fb_t *photo = capturePhoto();
    attachment.blob.data = photo->buf;
    attachment.blob.size = photo->len;

    attachment.descr.name = "Photo";
    attachment.descr.filename = "image/jpeg";
    attachment.descr.mime = "image/jpeg";

    // set message parameters
    message.sender.name    = "ESP32";
    message.sender.email   = SMTP_USER;
    message.subject        = "ESP32 Mail Client Test";
    message.addRecipient("Test", SMTP_DEST);
    message.addAttachment(attachment);

    // set message content
    String content       = "Hello, this is a photo taken with esp32 cam.";
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
        esp_restart();
    }
}

void setup()
{
    //disable brownout detector
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 

    // Initialize camera
    config.ledc_channel     = LEDC_CHANNEL_0;
    config.ledc_timer       = LEDC_TIMER_0;
    config.pin_d0           = Y2_GPIO_NUM;
    config.pin_d1           = Y3_GPIO_NUM;
    config.pin_d2           = Y4_GPIO_NUM;
    config.pin_d3           = Y5_GPIO_NUM;
    config.pin_d4           = Y6_GPIO_NUM;
    config.pin_d5           = Y7_GPIO_NUM;
    config.pin_d6           = Y8_GPIO_NUM;
    config.pin_d7           = Y9_GPIO_NUM;
    config.pin_xclk         = XCLK_GPIO_NUM;
    config.pin_pclk         = PCLK_GPIO_NUM;
    config.pin_vsync        = VSYNC_GPIO_NUM;
    config.pin_href         = HREF_GPIO_NUM;
    config.pin_sscb_sda     = SIOD_GPIO_NUM;
    config.pin_sscb_scl     = SIOC_GPIO_NUM;
    config.pin_pwdn         = PWDN_GPIO_NUM;
    config.pin_reset        = RESET_GPIO_NUM;
    config.xclk_freq_hz     = 20000000;
    config.pixel_format     = PIXFORMAT_JPEG;
    config.fb_location      = CAMERA_FB_IN_DRAM;
    config.frame_size       = FRAMESIZE_VGA;

    if (psramFound()) 
    {
        Serial.println("psram found");
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } 
    else 
    {
        Serial.println("psram NOT found");
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }
    // Camera init
    digitalWrite(PWDN_GPIO_NUM, LOW);
    delay(10);
    digitalWrite(PWDN_GPIO_NUM, HIGH);
    delay(10);
    esp_err_t err = esp_camera_init(&config);
    if (!esp_camera_init(&config)) 
    {
        Serial.printf("Camera init failed.");
        esp_restart();
    }

    // Initialize variables
    unsigned char attempts  = 0;
    bool led_on             = false;

    // Initialize serial 
    Serial.begin(115200);
    pinMode(LED, OUTPUT);

    // Initialize SPIFFS
    // if (!SPIFFS.begin(true))
    // {
    //     Serial.println("An Error has occurred while mounting SPIFFS");
    //     return;
    // }

    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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

    // Light the LED when connected
    blinkWhenReady();

    // Send email with photo
    sendPhoto();

    // Fix led when done
    digitalWrite(LED, HIGH);
}

void loop()
{
}