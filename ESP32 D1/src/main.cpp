#include <Arduino.h>

int led = 18 ;
void setup() {
  Serial.begin(9600);
  pinMode(led,OUTPUT);
  Serial.println("Hello World");
}

void loop() {
  digitalWrite(led, HIGH);
  delay(100);
  digitalWrite(led, LOW);
  delay(100);
  Serial.println("ESP32 OK");

}