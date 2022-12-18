//Libraries
#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <DHT_U.h>
#include <SPIFFS.h>
#include <FS.h>

//Create a Web Server object to listen for incoming HTTP requests on port 80.
AsyncWebServer server(80);

//Allows you to connect to the wifi point
const char *ssid = "Ordi de Gilles";
const char *password = "12345678";

const char* PARAM_INPUT_1 = "state"; 

//Pin for DHT11 + type of DHT
#define DHTPIN 26     // Digital pin connected to the DHT sensor
#define DHTTYPE    DHT11     // DHT 11
DHT dht(DHTPIN, DHTTYPE);

//Pin used
#define BP 25
#define LED 27

int ledState = LOW;          // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

//Read the temperature of DHT11
String readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT temp!");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}
//Read the state of the LED
String outputState(){
  if(digitalRead(LED)){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}

String processor(const String& var){
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }

  return String();
}


const char *PARAM_MESSAGE = "message";

//If no connection between the ESP32 and the web server then returns a 404 error. 
void notFound(AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Not found");
}

void setup(){
    Serial.begin(115200); //Setup the monitor serial

    //Defines I/O
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    pinMode(BP, INPUT);

    SPIFFS.begin(); //Initialize SPIFFS

    WiFi.mode(WIFI_STA); //ESP32 connects to an access point
    WiFi.begin(ssid, password); //Initialize the network settingd of the and provides the current status.
    if (WiFi.waitForConnectResult() != WL_CONNECTED){
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    /*Server.on defines a callback function. 
    *This registers a function that will be executed when a certain person moves to a later date.*/
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.css","text/css");
    });
    
    //Sends temperature data to the WebServer
    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", readDHTTemperature().c_str());
    });

    //Update of the LED status on the Webserver
    server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
        Serial.println("flag update");   
        String inputMessage;
        String inputParam;
        // GET input1 value on <ESP_IP>/update?state=<inputMessage>
        if (true/*/request->hasParam(PARAM_INPUT_1)/*/) {
          inputMessage = request->getParam(PARAM_INPUT_1)->value();
          inputParam = PARAM_INPUT_1;
          digitalWrite(LED, inputMessage.toInt());
          ledState = !ledState;
        }
        else {
          inputMessage = "No message sent";
          inputParam = "none";
        }
        Serial.println(inputMessage);
        request->send(200, "text/plain", "OK");
    });

    //Update of the button status on the Webserver
    server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
      request->send(200, "text/plain", String(digitalRead(LED)).c_str());
    });

    //Send a GET request to <IP>/get?message=<message>
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request){
      String message;
      if (request->hasParam(PARAM_MESSAGE)) {
        message = request->getParam(PARAM_MESSAGE)->value();
      } 
      else {
        message = "No message sent";
      }
      request->send(200, "text/plain", "Hello, GET: " + message); });

    //Send a POST request to <IP>/post with a form field message set to <message>
    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
      String message;
      if (request->hasParam(PARAM_MESSAGE, true)) {
        message = request->getParam(PARAM_MESSAGE, true)->value();
      } 
      else {
      message = "No message sent";
      }
      request->send(200, "text/plain", "Hello, POST: " + message); });

    server.onNotFound(notFound); //When there is no request from our client, it will send a 404 error message saying nothing found.
    server.begin(); // Start server
}

void loop(){
  //Read the state of the switch into a local variable:
  int reading = digitalRead(BP);
  /*Check to see if you just pressed the button
  *(i.e. the input went from LOW to HIGH), and you've waited long enough
  *since the last press to ignore any noise: */

  //If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    //Reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    /*Whatever the reading is at, it's been there for longer than the debounce
    *delay, so take it as the actual current state:*/

    //If the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      //Only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
      }
    }
  }
  //Set the LED:
  digitalWrite(LED, ledState);

  //Save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}