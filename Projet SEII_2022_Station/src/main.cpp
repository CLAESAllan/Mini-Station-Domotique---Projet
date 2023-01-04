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
int lastEtatPorte = 0;
int porteForce;
float lastTemp;
float lastHum;
float newTemp;
float newHum;


String CodeRecu;
String etatReset;
char json_DHT22[256];
char json_Reed[256];
char json_EtatPorte[256];
DynamicJsonDocument docDHT22(256);
DynamicJsonDocument docREED(256);
DynamicJsonDocument docEtatPorte(256);


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
  if(topic == "JSON_EtatCode"){
    //deserializeJson(docCodeValide, messageTemp);
    //std::string(CodeRecu) = docCodeValide["etatCode"];
    //CodeRecu = str(CodeRecuJSON);
    CodeRecu = messageTemp;
    //Serial.println(CodeRecu);
  }
  if(topic == "JSON_resetInfractionOUT"){
    etatReset = messageTemp;
  }
  if (topic == "BpManuelPorte"){
    porteForce = messageTemp.toInt();
    Serial.println(porteForce);
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
 //client.subscribe("EtatCode");
 client.subscribe("etatREED");
 //client.subscribe("resetInfractionOUT");
 client.subscribe("JSON_resetInfraction");
 client.subscribe("JSON_EtatCode");
 client.subscribe("BpManuelPorte");
 } else {
 Serial.print("failed, rc=");
 Serial.print(client.state());
 Serial.println(" try again in 5 seconds");
 // Wait 5 seconds before retrying

 delay(5000);
  }
 }
}

int checkDHT22(){
  if((newHum < lastHum - 0.5 || newHum > lastHum + 0.5) || (newTemp < lastTemp - 0.5 || newTemp > lastTemp + 0.5)){
    //Serial.println("DHT22 updt");
    return 1;
  }
  else{
    //Serial.println("DHT22 pas de updt");
    return 0;
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
 lastHum = dht.readHumidity(); // Read humidity in %
 lastTemp = dht.readTemperature(); //Read temperature as Celsius
}

void loop() {
//int etatReed = digitalRead(pinREED);
//StaticJsonDocument<200> doc;
 if (!client.connected()) {
  reconnect();
 }

if(!client.loop())
 client.connect(clientId.c_str(),"","095f3cdd2282");
 now = millis(); 
 delay(100);

/*/
  if(etatReed == LOW && lastCheck == 0 && etatReset == "Reset"){
    //cbon vois aimant donc fermer
    client.publish("etatREED","La porte est sécurisé");
    docREED["etatReed"] = etatReed;
    serializeJson(docREED,json_Reed);
    client.publish("JSON_REED", json_Reed);
    lastCheck = 1;
  }else if (etatReed == HIGH && CodeRecu == "pasOK" && lastCheck == 1){
    client.publish("etatREED","!!! INFRACTION !!!");
    docREED["etatReed"] = etatReed;
    serializeJson(docREED,json_Reed);
    client.publish("JSON_REED", json_Reed);
    etatReset = "pasReset";
    lastCheck = 0 ;
  }
/*/

  //Ajouter ouverture manuelle
  if((CodeRecu == "pasOK") && (porteForce == 0)/*/&& lastEtatPorte == 0/*/){
    myservo.write(95);
    //client.publish("JSON_EtatPorte", "Porte fermée");
    docEtatPorte["etatPorte"] = "Porte fermée";
    serializeJson(docEtatPorte, json_EtatPorte);
    client.publish("JSON_EtatPorte", json_EtatPorte);
    lastEtatPorte = 1; 
  }
  else if ((CodeRecu == "OK") || (porteForce == 1)){
    myservo.write(0);
    //client.publish("JSON_EtatPorte", "Porte ouverte");
    docEtatPorte["etatPorte"] = "Porte ouverte";
    serializeJson(docEtatPorte, json_EtatPorte);
    client.publish("JSON_EtatPorte", json_EtatPorte);
    lastEtatPorte = 0;
  }

//Publishes new temperature and humidity every 3 seconds
if (now - lastMeasure > 3000) {
 lastMeasure = now;
 
 float h = dht.readHumidity(); // Read humidity in %
 float t = dht.readTemperature(); //Read temperature as Celsius
 newHum = h;
 newTemp = t;
 //static char tCHAR[3];
 //dtostrf(t,3,1,tCHAR); //Function dtostrf converts double and floating-point values into a string.

  //Check if any reads failed and exit early (to try again).
 if (isnan(h) || isnan(t)) {
 Serial.println("Failed to read from DHT sensor!");
 return;
 }

/*/if (checkDHT22()){/*/

//Computes temperature values in Celsius
//float hic = dht.computeHeatIndex(t, h, false);

 static char temperatureTemp[7];
 dtostrf(t, 6, 2, temperatureTemp); //Function dtostrf converts double and floating-point values into a string.

 static char humidityTemp[7];
 dtostrf(h, 6, 2, humidityTemp); //Function dtostrf converts double and floating-point values into a string.
 


 //Publishes Temperature and Humidity values

 //client.publish("temperature", temperatureTemp);
 //client.publish("humidite", humidityTemp);

 //DynamicJsonDocument doc(256);

  //We could then access the data inside it using the same dot/bracket
  docDHT22["temperature"] = temperatureTemp;
  docDHT22["humidite"] = humidityTemp;
  // Generate the minified JSON and send it to the Serial port.
  serializeJson(docDHT22, json_DHT22);
  Serial.println(json_DHT22);
  client.publish("JSON_DHT22",json_DHT22);

  // Generate the prettified JSON and send it to the Serial port.
  //serializeJsonPretty(doc, Serial);
    /*/}/*/
  }
}