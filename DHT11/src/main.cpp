#include "DHT.h"

#define DHTPIN  14
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() 
{
    Serial.begin(9600);
    dht.begin();
    delay(2000);
}

void loop() 
{
    // Reading temperature or humidity takes about 250 milliseconds!
    float h = dht.readHumidity();

    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h)) 
    {
        Serial.println(F("Failed to read humidity from DHT sensor!"));
        delay(2500);
        return;
    }
    if (isnan(t)) 
    {
        Serial.println(F("Failed to read temperature from DHT sensor!"));
        delay(2500);
        return;
    }

    // Print Results
    Serial.print(F("Humidity : "));
    Serial.print(h);
    Serial.print(F("%  Temperature : "));
    Serial.print(t);
    Serial.print(F("°C\n"));

    // Wait a few seconds between measurements.
    delay(3000);
}