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

const char *ssid = "Ordi de Gilles";
const char *password = "12345678";

const char *PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

void setup()
{

    Serial.begin(9600);

    /*/ Initialize LittleFS
    if (!LITTLEFS.begin())
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }
    /*/

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
        SPIFFS.begin();
        File f = SPIFFS.open("/index.html","r");
        request->send(200, "text/plain", f);
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
{/*/
    SPIFFS.begin();
    File f = SPIFFS.open("/index.html","r");
        if (!f){
            Serial.print("Fichier ouverture raté");
        }
    f.close();
    /*/
}