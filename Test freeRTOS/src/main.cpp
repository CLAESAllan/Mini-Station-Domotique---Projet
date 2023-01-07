#include <Arduino.h>
#include <Keypad.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

const char *SSID = "Ordi de Gilles"; // Nom du réseau WiFi
const char *PASSWORD = "12345678"; // Mot de passe du réseau WiFi
const char *SERVER = "192.168.137.1"; // Adresse IP du serveur Node-RED
const int PORT = 1883; // Port du serveur MQTT
String clientId = "ello";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

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
}

void wifiTask(void *pvParameters) {

  vTaskDelay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500);
    Serial.print(".");
  }
 Serial.println("");
 Serial.print("WiFi connected - ESP IP address: ");
 Serial.println(WiFi.localIP());
  
  while (!mqttClient.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 
 if (mqttClient.connect(clientId.c_str(),"","095f3cdd2282")) {
 Serial.println("connected");

  // Connexion réussie
  Serial.println("Connected to MQTT server!");
  } else {
 Serial.print("failed, rc=");
 Serial.print(mqttClient.state());
 Serial.println(" try again in 5 seconds");
 // Wait 5 seconds before retrying
 vTaskDelay(5000);
    }

  }
  vTaskDelete(NULL);
}

void dataTask(void *pvParameters) {
  while (true) {
    // Création d'un objet JSON pour stocker les données
    DynamicJsonDocument doc(256);
    doc["temperature"] = 25.0;
    doc["humidity"] = 50.0;

    // Conversion de l'objet JSON en chaîne de caractères
    char payload[256];
    serializeJson(doc, payload);

    // Envoi des données au serveur via MQTT
    mqttClient.publish("sensor/data", payload);

    // Pause de 5 secondes avant l'envoi de nouvelles données
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200); // Initialisation de la communication série
  xTaskCreate(wifiTask, "mqttTask", 4096,NULL,1,NULL);
  xTaskCreate(dataTask, "dataTask", 4096, NULL,2, NULL);
  // Configuration de l'objet MQTTClient
  mqttClient.setServer(SERVER, PORT);
  mqttClient.setCallback(callback);

  // Création de la tâche "wifiTask" avec une priorité de 1
  

  // Création de la tâche "dataTask" avec une priorité de 2
  
}

void loop() {
}