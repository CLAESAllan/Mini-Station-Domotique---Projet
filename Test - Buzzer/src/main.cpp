#include <Arduino.h>
#include <ESP32Tone.h>

#define BUZZZER_PIN  32 // ESP32 pin GIOP18 connected to piezo buzzer

int boucleBuzz = 1000;


void setup() {
 pinMode(BUZZZER_PIN, OUTPUT);
 
}

void loop() {
while(boucleBuzz < 4000 ){
      tone(BUZZZER_PIN, boucleBuzz,125);
      boucleBuzz +=500;
      }  
    boucleBuzz = 100;  
}