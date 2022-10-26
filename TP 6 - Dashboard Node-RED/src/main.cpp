#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include <analogWrite.h>
#include <Adafruit_NeoPixel.h>

#define DHTTYPE DHT11

const char* ssid = "Ordi de Gilles";
const char* password = "12345678";

const char* mqtt_server = "192.168.137.1";  //192.168.137.1   172.18.0.3

WiFiClient espClient;
PubSubClient client(espClient);

#define DHTPin 26
#define BP 12
#define RGB 0
#define LED 32


char button[6]; // trame for data to be send

bool etatLED = LOW; // the current state of LED
int etatBP; // the current state of button
int dernierEtatBP;// the previous state of button

int valeurR;
int valeurG;
int valeurB;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, RGB, NEO_GRB + NEO_KHZ800);
#define BRIGHTNESS 50 // Set BRIGHTNESS to about 1/5 (max = 255)

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);
// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;
String clientId = "Hello";

void setup_wifi() {
 delay(10);
 // We start by connecting to a WiFi network
 Serial.println();
 Serial.print("Connecting to ");
 Serial.println(ssid);
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
 Serial.println("");
 Serial.print("WiFi connected - ESP IP address: ");
 Serial.println(WiFi.localIP());
}

void callback(String topic, byte* message, unsigned int length) {
 Serial.print("Message arrived on topic: ");
 Serial.print(topic);
 Serial.print(". Message: ");
 String messageTemp;

 for (int i = 0; i < length; i++) {
 Serial.print((char)message[i]);
 messageTemp += (char)message[i];
 }
 Serial.println();
 
 if(topic=="LED"){
  Serial.print("Changing Room lamp to ");
 if(messageTemp == "on"){
  digitalWrite(LED, HIGH);
  Serial.print("On");
 }
 else if(messageTemp == "off"){
  digitalWrite(LED, LOW);
  Serial.print("Off");
 }
}

 if(topic=="valeurR")
 {
 valeurR = messageTemp.toInt();
  Serial.println(valeurR);
 }
 if(topic=="valeurG")
 {
 valeurG = messageTemp.toInt();
  Serial.println(valeurG);
 }
 if(topic=="valeurB")
 {
 valeurB = messageTemp.toInt();
  Serial.println(valeurB);
 }
 Serial.println();
}

void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 /*
 YOU MIGHT NEED TO CHANGE THIS LINE,
 IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
 To change the ESP device ID,
 you will have to give a new name to the ESP32.
 Here's how it looks:
 if (client.connect("ESP32Client")) {
  You can do it like this:
 if (client.connect("ESP1_Office")) {
 Then, for the other ESP:
 if (client.connect("ESP2_Garage")) {
 That should solve your MQTT multiple connections problem
 */
 if (client.connect(clientId.c_str(),"","095f3cdd2282")) {
 Serial.println("connected");
 
 client.subscribe("LED");
 //subscribe to topic room_R
 client.subscribe("valeurR");
 //subscribe to topic room_G
 client.subscribe("valeurG");
 //subscribe to topic room_B
 client.subscribe("valeurB");
 client.subscribe("JSON");
 } else {
 Serial.print("failed, rc=");
 Serial.print(client.state());
 Serial.println(" try again in 5 seconds");
 // Wait 5 seconds before retrying
 delay(5000);
 }
 }
}


void setup() {
 // Initialize pins used
 strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'
 pinMode(LED, OUTPUT);
 pinMode(BP, INPUT);

 dht.begin();// start module DHT11
 Serial.begin(115200);// initialize serial monitor
 setup_wifi();//
 // set server mqtt on port 1883 to the client
 client.setServer(mqtt_server, 1883);
 // set the function callback to the client
 client.setCallback(callback);

}

void loop() {
StaticJsonDocument<200> doc;
 if (!client.connected()) {
  reconnect();
 }
if(!client.loop())
 client.connect(clientId.c_str(),"","095f3cdd2282");
 now = millis();
strip.fill(strip.Color(valeurR, valeurG, valeurB, BRIGHTNESS));
strip.show();

 etatBP = digitalRead(BP);// read new state
 if (dernierEtatBP == HIGH && etatBP == LOW) {
  etatLED = !etatLED;
  digitalWrite(LED, etatLED);
  snprintf(button, 6, "%u", etatLED);
  client.publish("BP", button);
 }
 dernierEtatBP = etatBP;// Update state toggle
 delay(50);



 if (now - lastMeasure > 3000) {
 lastMeasure = now;
 
 float h = dht.readHumidity();
 float t = dht.readTemperature();
 static char tCHAR[3];
 dtostrf(t,3,1,tCHAR);

 if (isnan(h) || isnan(t)) {
 Serial.println("Failed to read from DHT sensor!");
 return;
 }

 float hic = dht.computeHeatIndex(t, h, false);
 static char temperatureTemp[7];
 dtostrf(hic, 6, 2, temperatureTemp);

 // Uncomment to compute temperature values in Fahrenheit
 // float hif = dht.computeHeatIndex(f, h);
 // static char temperatureTemp[7];
 // dtostrf(hif, 6, 2, temperatureTemp);

 static char humidityTemp[7];
 dtostrf(h, 6, 2, humidityTemp);
 
 client.publish("temperature", temperatureTemp);
 client.publish("humidite", humidityTemp);

 Serial.print("Humidity: ");
  Serial.print(h);
 Serial.print(" %\t Temperature: ");
 Serial.print(t);
 Serial.print(" *C ");
  doc["temperature"] = tCHAR;
  doc["humidite"] = h;
  doc["etatBP"] = button;

  // Generate the minified JSON and send it to the Serial port.
  //
  serializeJson(doc, Serial);
  static char jsonCHAR [100];
  serializeJson(doc, jsonCHAR);

  client.publish("JSON",jsonCHAR );
  // Start a new line
  Serial.println();

  // Generate the prettified JSON and send it to the Serial port.
  //
  serializeJsonPretty(doc, Serial);
  }
}