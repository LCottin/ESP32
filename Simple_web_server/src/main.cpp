#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "CONFIGS.hpp"

const char  led      = 2;
const char* ssid     = SSID;
const char* password = PASSWORD;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char root_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
    <title>ESP32 Web Server</title>
</head>
<body>
    <h2>
        ESP32 Server
    </h2>
    <p>
        This is my first server on ESP32 !
    </p>
</body>
</html>)rawliteral";

const char welcome_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
    <title>ESP32 Web Server</title>
</head>
<body>
    <h2>
        ESP32 Server
    </h2>
    <p>
        WELCOME !
    </p>
</body>
</html>)rawliteral";

void setup()
{
    Serial.begin(115200);
    pinMode(led, OUTPUT);

    bool led_on = false;

    // Connect to Wi-Fi 
    unsigned attempts = 0;
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(1000);
        Serial.println("Connecting to WiFi.. Attempt " + String(++attempts));
        digitalWrite(led, led_on);
        led_on = !led_on;
    }

    // Light the LED when connected
    digitalWrite(led, HIGH);

    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200, "text/html", root_html);
    });

    // Route for welcome /welcome web page
    server.on("/welcome", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200, "text/html", welcome_html);
    });

    // Start server
    server.begin();
}
 
void loop()
{  
}