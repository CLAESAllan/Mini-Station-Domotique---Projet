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

AsyncWebServer server(80);

const char *ssid = "DESKTOP-JFITQ99";
const char *password = "n9C2247|";
const char* PARAM_INPUT_1 = "state";

#define DHTPIN 26     // Digital pin connected to the DHT sensor
#define DHTTYPE    DHT11     // DHT 11
DHT dht(DHTPIN, DHTTYPE);

#define BP 25
#define LED 27

int ledState = LOW;          // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

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
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  
 /*/ if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    String outputStateValue = outputState();
    buttons+= "<p4>LED<span id=\"outputState\"></span></p4><label class=\"switch\"><input type=\"button\" onclick=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue ;//+ ">//<span class=\"slider\"></span></label>"
    return buttons;
  }/*/
  return String();
}


const char *PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void setup()
{

    Serial.begin(115200); 
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    pinMode(BP, INPUT);
    SPIFFS.begin();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Route for root / web page

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", String(), false, processor);
    });

    server.on("/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.css","text/css");
    });

    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });

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

server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
  request->send(200, "text/plain", String(digitalRead(LED)).c_str());
});


    // Send a GET request to <IP>/get?message=<message>
    server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String message;
        if (request->hasParam(PARAM_MESSAGE)) {
            message = request->getParam(PARAM_MESSAGE)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, GET: " + message); });

    // Send a POST request to <IP>/post with a form field message set to <message>
    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request)
              {
        String message;
        if (request->hasParam(PARAM_MESSAGE, true)) {
            message = request->getParam(PARAM_MESSAGE, true)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, POST: " + message); });

    server.onNotFound(notFound);

    server.begin();
}

void loop()
{
// read the state of the switch into a local variable:
  int reading = digitalRead(BP);
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        ledState = !ledState;
      }
    }
  }
  // set the LED:
  digitalWrite(LED, ledState);

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}