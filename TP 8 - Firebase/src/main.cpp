//Libraries
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <Firebase.h>
#include <DHT_U.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <addons/TokenHelper.h> //Provide the token generation process info.
#include <addons/RTDBHelper.h> //Provide the RTDB payload printing info and other helper functions.

// Insert your network credentials
#define WIFI_SSID "Ordi de Gilles"
#define WIFI_PASSWORD "12345678"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDvQePS4aEx3PxM1zXMibI6zVVGbaIqLvE"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://tp8---firebase-default-rtdb.europe-west1.firebasedatabase.app/" 

//Definitions of variables
#define LED1 32
#define LED2 5
#define BP 12
#define DHTPIN 26
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Define Firebase Data object
FirebaseData fbdo; //Firebase Realtime Database Object
FirebaseData stream;

FirebaseAuth auth; //Firebase Authentication Object
FirebaseConfig config; //Firebase configuration Object

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
volatile bool dataChanged = false;


void streamCallback(FirebaseStream data){
  Serial.printf("sream path, %s\nevent path, %s\ndata type, %s\nevent type, %s\n\n",
                data.streamPath().c_str(),
                data.dataPath().c_str(),
                data.dataType().c_str(),
                data.eventType().c_str());
  printResult(data); // see addons/RTDBHelper.h
  Serial.println();

  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", data.payloadLength(), data.maxPayloadLength());
  dataChanged = true;
}

void streamTimeoutCallback(bool timeout){
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

void setup(){
  Serial.begin(115200); //Initialise serial communication 
  dht.begin(); //Initialize module DHT11

  // Initialize pins used
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BP, INPUT);

  //Define the WiFi credentials
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

if (!Firebase.RTDB.beginStream(&stream, "/test/stream/data"))
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());

  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

}

void loop(){

  float h = dht.readHumidity(); //Read Humidity
  float t = dht.readTemperature(); //Read temperature

  if (isnan(h) || isnan(t)) {
    return;
  }
  
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    //Write an Int number on the database path test/int
    if (Firebase.RTDB.getInt(&fbdo, "/LED 1")){
      if (fbdo.dataType() == "int"){
       int EtatLED1 = fbdo.intData();
       digitalWrite(LED1, EtatLED1);
      }
      
    }
    else {
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    //Write an Int number on the database path test/int
    if (Firebase.RTDB.getInt(&fbdo, "/LED 2")){
      if (fbdo.dataType() == "int"){
       int EtatLED2 = fbdo.intData();
       digitalWrite(LED2, EtatLED2);
      }
    }
    else {
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }

  bool EtatBP = digitalRead(BP);
  if (Firebase.RTDB.setBool(&fbdo, "/Etat BP", EtatBP)){

  }
  else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }

  if (dataChanged){
    dataChanged = false;
    // When stream data is available, do anything here...
  }
}