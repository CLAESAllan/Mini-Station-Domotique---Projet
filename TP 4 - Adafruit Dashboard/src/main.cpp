#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <AdafruitIO_WiFi.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <DHT_U.h>
#include <analogWrite.h>
#include <Adafruit_NeoPixel.h>

//Definition of variables
#define LED 32
#define DHTPIN 26
#define DHTTYPE DHT11
#define LDR 39
#define RGB 0
#define Bouton 12
DHT dht(DHTPIN, DHTTYPE);
int Luminosite;
int Rouge = 0, Vert = 0, Bleu = 0;
int EtatBouton;
int readingR;
int readingV;
int readingB;

//Definition of WiFi
#define WIFI_SSID       "Ordi de Gilles"
#define WIFI_PASS       "12345678"

//Defintion of your key on adafruitIo
#define IO_USERNAME  "Gilles_Sl"
#define IO_KEY       "aio_EgAO2602GvXVxXkhscXEq7JCBEai"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

//Call for seeds on adafruitIO
AdafruitIO_Feed *led = io.feed("LED");
AdafruitIO_Feed *temperature = io.feed("Temperature");
AdafruitIO_Feed *humidity = io.feed("Humidite");
AdafruitIO_Feed *luminosity = io.feed("Luminosite");
AdafruitIO_Feed *textBox = io.feed("Etat BP");
AdafruitIO_Feed *sliderRed = io.feed("SliderRed");
AdafruitIO_Feed *sliderGreen = io.feed("Slider Vert");
AdafruitIO_Feed *sliderBlue = io.feed("Slider Bleu");

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT  1

// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 25 // Set BRIGHTNESS to about 1/5 (max = 255)

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, RGB, NEO_GRB + NEO_KHZ800);

//Creating the LED function
void handleMessageLED(AdafruitIO_Data *data){
  if(data->toPinLevel() == HIGH){
  }
  else{
  }
  Serial.println("FLAG BP");
  Serial.println(data->toPinLevel());
  digitalWrite(LED, data->toPinLevel());      //Write the current state to the led
}

//Creating the red colour function of the RGB LED
void handleMessageRed(AdafruitIO_Data *data){
  readingR = data->toInt();                //Convert the data to integer

}

//Creating the bluecolour function of the RGB LED
void handleMessageBlue(AdafruitIO_Data *data){
  readingB = data->toInt();                //Convert the data to integer
}


//Creating the green colour function of the RGB LED
void handleMessageGreen(AdafruitIO_Data *data){
  readingV = data->toInt();                //Convert the data to integer
}



void setup() {

  pinMode(LED, OUTPUT);       //Defines LED as output

  Serial.begin(115200);       //Definition of the data rate

  while(! Serial);            //Wait for serial monitor to open
  dht.begin();

  Serial.print("Connecting to Adafruit IO");
  io.connect();               //Connect to io.adafruit.com
  
  led->onMessage(handleMessageLED);           //Set up a message handler for the 'led' feed. Will be called whenever a message is received from adafruit io.
  sliderRed->onMessage(handleMessageRed);     //Set up a message handler for the 'sliderRed' feed. Will be called whenever a message is received from adafruit io.
  sliderBlue->onMessage(handleMessageBlue);   //Set up a message handler for the 'sliderBlue' feed. Will be called whenever a message is received from adafruit io.
  sliderGreen->onMessage(handleMessageGreen); //Set up a message handler for the 'sliderGreen' feed. Will be called whenever a message is received from adafruit io.

  //Defines Pin RGB as output
  pinMode(RGB, OUTPUT);

  //Wait for a connection
  while(io.status() < AIO_CONNECTED) {
    delay(500);
  }

  led->get();
  sliderRed->get();
  sliderGreen->get();
  sliderBlue->get();
  
}

void loop() {

  io.run();                             //Maintains client connection to adafruitIO

  float h = dht.readHumidity();         // Read humidityas
  float t = dht.readTemperature();      // Read temperature as Celsius (the default)

  if(isnan(h) || isnan(t)){             //Check if any reads failed and exit early (to try again).
    return;
  }

  int LDRStatus = analogRead(LDR);      //Transforms the analog value of the LDR to int
  Luminosite = map(LDRStatus, 0, 4095, 0, 100);   //Mapping the int value 

  EtatBouton = digitalRead(Bouton);     //Read button value
  
  //Grab the state of the "EtatBouton" button
  if ( EtatBouton == LOW){              //If the state is low then we send an "OFF"
    textBox->save("OFF");
  }
  else{                                 //Otherwise an "ON
    textBox->save("ON");
  }
  
  //temperature->save(t);                 //Save the value of t in the temperature Feed on adafruitIO
  //humidity->save(h);                    //Save the value of h in the humidity feed on adafruitIO
  //luminosity->save(Luminosite);         //Save the value of Luminosity in the luminosity on adafruitIO
  strip.fill(strip.Color(readingR, readingV, readingB, BRIGHTNESS));
  strip.show();
  delay(5000);
}