#include <Arduino.h>
// for Arduino microcontroller
int digitalPin = 2;
int NombrePassage = 0;
// for ESP8266 microcontroller
//int ledPin = D7; 
//int digitalPin = D8;

// for ESP32 microcontroller
//int ledPin = 0; 
//int digitalPin = 2;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int digitalVal = digitalRead(digitalPin);
  
  if (digitalVal == LOW) {
    NombrePassage += 1;
    Serial.println(NombrePassage);
  }
  
  delay(100);
}