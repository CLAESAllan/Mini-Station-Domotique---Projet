//Library
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP32Servo.h>

//Definition of variables
#define DHTPin 4
#define DHTTYPE DHT22 //type of sensor
#define BP 12
#define LED 32

int pinREED = 19;
int NombrePassage = 0;
int pos = 0;    // variable to store the servo position
int lastCheck = 0;
String CodeRecu = "pasOK";
String etatReset = "Reset";

//Change the credentials below, so your ESP32 connects to your router
const char* ssid = "Ordi de Gilles";
const char* password = "12345678";

//Connect to your MQTT broker
const char* mqtt_server = "192.168.137.1";  //192.168.137.1   172.18.0.3

//Initializes the espClient.
WiFiClient espClient;
PubSubClient client(espClient);

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

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

/*This function is performed when a device sends 
 *a message to a subject to which your ESP32 is subscribed.
 *Let's modify the function below to add program logic, 
 *so that when the device posts to a topic that your ESP32 is subscribed to.
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
  if(topic == "EtatCode"){
    CodeRecu = messageTemp;
  }
  if(topic == "resetInfractionOUT"){
    etatReset = messageTemp;
  }
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
 client.subscribe("EtatCode");
 client.subscribe("etatREED");
 client.subscribe("resetInfractionOUT");
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

 myservo.attach(13);  // attaches the servo on pin 13 to the servo object
 
 dht.begin();//  module DHT11

 Serial.begin(115200); //Initialize serial monitor
 setup_wifi();
 // set server mqtt on port 1883 to the client
 client.setServer(mqtt_server, 1883);
 // set the function callback to the client
 client.setCallback(callback);
}

void loop() {
int etatReed = digitalRead(pinREED);
StaticJsonDocument<200> doc;
 if (!client.connected()) {
  reconnect();
 }
if(!client.loop())
 client.connect(clientId.c_str(),"","095f3cdd2282");
 now = millis();
  
  /*/
  if (digitalVal == LOW) {
    NombrePassage += 1;
    Serial.println(NombrePassage);
  }
  /*/

  if(etatReed == LOW && lastCheck == 0 && etatReset == "Reset"){
    //cbon vois aimant donc fermer
    client.publish("etatREED","La porte est sécurisé");
    lastCheck = 1;
  }else if (etatReed == HIGH && CodeRecu == "pasOK" && lastCheck == 1){
    client.publish("etatREED","!!! INFRACTION !!!");
    etatReset = "pasReset";
    lastCheck = 0 ;
  }

  if(CodeRecu == "pasOK"){
    myservo.write(95); 
  }
  else if (CodeRecu == "OK"){
    myservo.write(0); 
  }
  delay(100);

//Publishes new temperature and humidity every 3 seconds
 if (now - lastMeasure > 3000) {
 lastMeasure = now;
 
 float h = dht.readHumidity(); // Read humidity in %
 float t = dht.readTemperature(); //Read temperature as Celsius
 static char tCHAR[3];
 dtostrf(t,3,1,tCHAR); //Function dtostrf converts double and floating-point values into a string.

  //Check if any reads failed and exit early (to try again).
 if (isnan(h) || isnan(t)) {
 Serial.println("Failed to read from DHT sensor!");
 return;
 }

//Computes temperature values in Celsius
 float hic = dht.computeHeatIndex(t, h, false);
 static char temperatureTemp[7];
 dtostrf(hic, 6, 2, temperatureTemp); //Function dtostrf converts double and floating-point values into a string.

 static char humidityTemp[7];
 dtostrf(h, 6, 2, humidityTemp); //Function dtostrf converts double and floating-point values into a string.
 
 //Publishes Temperature and Humidity values
 client.publish("temperature", temperatureTemp);
 client.publish("humidite", humidityTemp);

/*/
 Serial.print("Humidity: ");
 Serial.print(h);
 Serial.print(" %\t Temperature: ");
 Serial.print(t);
 Serial.print(" *C ");
/*/
  //We could then access the data inside it using the same dot/bracket
  doc["temperature"] = tCHAR;
  doc["humidite"] = h;

  // Generate the minified JSON and send it to the Serial port.
  //serializeJson(doc, Serial);
  static char jsonCHAR [100];
  //serializeJson(doc, jsonCHAR);

  client.publish("JSON",jsonCHAR );
  // Start a new line
  //Serial.println();

  // Generate the prettified JSON and send it to the Serial port.
  //serializeJsonPretty(doc, Serial);
  }
}