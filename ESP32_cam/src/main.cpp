#include <Arduino.h>

// ledPin refers to ESP32-CAM GPIO 4 (flashlight)
const int ledPin = 4;

void setup() {
  // initialize digital pin ledPin as an output
  pinMode(ledPin, OUTPUT);
}

void loop() {
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(1000);
}