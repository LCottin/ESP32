#include <Arduino.h>

#define LED     2
#define DELAY   1000

void setup() 
{
    pinMode(LED, OUTPUT);
}

void loop() 
{
    delay(DELAY);
    digitalWrite(LED, HIGH);
    delay(DELAY);
    digitalWrite(LED, LOW);
}