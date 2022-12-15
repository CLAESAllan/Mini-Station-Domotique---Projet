//Library
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include <analogWrite.h>
#include <Adafruit_NeoPixel.h>

//Definition of variables
#define DHTPin 26
#define DHTTYPE DHT11
#define BP 12
#define RGB 0
#define LED 32

const char* ssid = "Ordi de Gilles";
const char* password = "12345678";

const char* mqtt_server = "192.168.137.1";  //192.168.137.1   172.18.0.3

//Initializes the espClient.
WiFiClient espClient;
PubSubClient client(espClient);

char button[6]; // trame for data to be send

bool etatLED = LOW; // the current state of LED
int etatBP; // the current state of button
int dernierEtatBP;// the previous state of button

int valeurR; // Variable to stock value Red reveived
int valeurG; // Variable to stock value Green reveived
int valeurB; // Variable to stock value Blue reveived

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, RGB, NEO_GRB + NEO_KHZ800);
#define BRIGHTNESS 50 // Set BRIGHTNESS to about 1/5 (max = 255)

// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);
// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;
String clientId = "Hello";

//This functions connects your ESP32 to your router
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

/*This functions is executed when some device publishes a message to a topic that your ESP32 is subscribed to 
 *Change the function below to add logic to your program, 
 *so when a device publishes a message to a topic that 
 *your ESP32 is subscribed you can actually do something
 */
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
 
 /* If a message is received on the topic LED,
 * you check if the message is either on or off.
 * Turns the LED GPIO according to the message
 */
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

/*If a message is received on the topic valeurR
 * write the values to GPIO according to the message
 * it is for the control of lED RGB (color Red)
 */
 if(topic=="valeurR")
 {
 valeurR = messageTemp.toInt();
  Serial.println(valeurR);
 }

 /*If a message is received on the topic valeurG
 * write the values to GPIO according to the message
 * it is for the control of lED RGB (color Green)
 */
 if(topic=="valeurG")
 {
 valeurG = messageTemp.toInt();
  Serial.println(valeurG);
 }

 /*If a message is received on the topic valeurB
 * write the values to GPIO according to the message
 * it is for the control of lED RGB (color Blue)
 */
 if(topic=="valeurB")
 {
 valeurB = messageTemp.toInt();
  Serial.println(valeurB);
 }
 Serial.println();
}

/* This functions reconnects your ESP32
* to your MQTT broker */
void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 
 if (client.connect(clientId.c_str(),"","095f3cdd2282")) {
 Serial.println("connected");
 
 client.subscribe("LED");
 //subscribe to topic valeurR
 client.subscribe("valeurR");
 //subscribe to topic valeurG
 client.subscribe("valeurG");
 //subscribe to topic valeurB
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
 pinMode(LED, OUTPUT);
 pinMode(BP, INPUT);
  
 strip.begin();
 strip.setBrightness(50);
 strip.show(); // Initialize all pixels to 'off'
 
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

//Toggle Button
 etatBP = digitalRead(BP);// read new state
 if (dernierEtatBP == HIGH && etatBP == LOW) {
  etatLED = !etatLED;
  //Control LED arccoding to the toggled state
  digitalWrite(LED, etatLED);
  //Convert to a data that could be send
  snprintf(button, 6, "%u", etatLED);
  //Send to the topic BP 
  client.publish("BP", button);
 }
 dernierEtatBP = etatBP;// Update state toggle
 delay(50);

//Publishes new temperature and humidity every 3 seconds
 if (now - lastMeasure > 3000) {
 lastMeasure = now;
 
 float h = dht.readHumidity(); // Read temperature as Celsius
 float t = dht.readTemperature();
 static char tCHAR[3];
 dtostrf(t,3,1,tCHAR);

  //Check if any reads failed and exit early (to try again).
 if (isnan(h) || isnan(t)) {
 Serial.println("Failed to read from DHT sensor!");
 return;
 }

//Computes temperature values in Celsius
 float hic = dht.computeHeatIndex(t, h, false);
 static char temperatureTemp[7];
 dtostrf(hic, 6, 2, temperatureTemp);

 static char humidityTemp[7];
 dtostrf(h, 6, 2, humidityTemp);
 
 //Publishes Temperature and Humidity values
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
  serializeJson(doc, Serial);
  static char jsonCHAR [100];
  serializeJson(doc, jsonCHAR);

  client.publish("JSON",jsonCHAR );
  // Start a new line
  Serial.println();

  // Generate the prettified JSON and send it to the Serial port.
  serializeJsonPretty(doc, Serial);
  }
}