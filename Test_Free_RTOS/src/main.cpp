#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP32Servo.h>

#define DHTTYPE DHT22
#define DHTPIN 4
#define REED_PIN 19

TaskHandle_t Task1;
TaskHandle_t Task2;                                               

const char *SSID = "Ordi de Gilles"; // Nom du réseau WiFi
const char *PASSWORD = "12345678"; // Mot de passe du réseau WiFi
const char *SERVER = "192.168.137.1"; // Adresse IP du serveur Node-RED
const int PORT = 1883; // Port du serveur MQTT

String clientId = "Hello";
String CodeRecu;
String etatReset;

int porteForce;
int lastEtatPorte = 0;

DynamicJsonDocument docDHT22(256);
DynamicJsonDocument docEtatPorte(256);

char json_DHT22 [256];
char json_EtatPorte[256];

DHT dht(DHTPIN, DHTTYPE);

Servo servo;

WiFiClient espClient;                                             // Création de "espClient" propre au client Wi-Fi
PubSubClient client(espClient); 

void setup_wifi() {                                               // Fonction de connection au WiFi

  delay(10);                                                      // Délai de 10ms
  Serial.println();                                               // Imprime un retour à la ligne dans la console
  Serial.print("Connecting to ");                                 // Imprime un message dans la console
  Serial.println(SSID);                                           // Imprime dans la console le nom du WiFi

  WiFi.mode(WIFI_STA);                                            // Execution du type de mode de la connexion WiFi
  WiFi.begin(SSID, PASSWORD);                                     // Lancement de la connexion WiFi

  while (WiFi.status() != WL_CONNECTED) {                         // Tant que le microcontrôleur n'est pas connecté au WiFi
    delay(500);                                                   // Délai de 500ms
    Serial.print(".");                                            // Imprimme un point dans la console
  }

  randomSeed(micros());                                           // Création d'une "clef" aléatoire

  Serial.println("");                                             // Imprime un message dans la console
  Serial.println("WiFi connected");                               // Imprime un message dans la console
  Serial.println("IP address: ");                                 // Imprime un message dans la console
  Serial.println(WiFi.localIP());                                 // Affichage dans la console que l'on est connecté avec cette adresse IP
}

void callback(char* topic, byte* payload, unsigned int length) {  // Fonction de Callback quand un message MQTT arrive sur un topic (subscribe)
  Serial.print("Message arrived [");                              // Imprime un message dans la console
  Serial.print(topic);                                            // On affiche de quel topic il s'agit
  Serial.print("] ");                                             // Imprime un message dans la console
  String messageTemp;                                            
  for (int i = 0; i < length; i++) {                              // On boucle tous les caractères reçus sur le topic
    Serial.print((char)payload[i]);                               // On affiche un à un tous les caractères reçus
  }
  Serial.println();                                               // Imprime un retour à la ligne dans la console
  if(topic == "JSON_EtatCode"){
    CodeRecu = messageTemp;
  }
  if(topic == "JSON_resetInfractionOUT"){
    etatReset = messageTemp;
  }
  if (topic == "BpManuelPorte"){
    porteForce = messageTemp.toInt();
    Serial.println(porteForce);
  }
}

void reconnect() {                                                // Fonction effectuant des tentative de reconnexion 
  while (!client.connected()) {                                   // Si le client se connecte
    Serial.print("Attempting MQTT connection...");                // On attent la conexion MQTT
    if (client.connect("ESP32")) {                                // Si le client se connecte en étant appelé "ESP32"
      Serial.println("connected");
      //client.subscribe("EtatCode");
      client.subscribe("etatREED");
      //client.subscribe("resetInfractionOUT");
      client.subscribe("JSON_resetInfraction");
      client.subscribe("JSON_EtatCode");
      client.subscribe("BpManuelPorte");
      } 
    else {                                                        // Si la connexion rate
      Serial.print("failed, rc=");                                // On affiche qu'il y a une erreur
      Serial.print(client.state());                               // On affiche le numéro de l'erreur
      Serial.println(" try again in 5 seconds");                  // On affiche comme quoi on réessaye
      delay(5000);                                                // On attend 5 secondes avant de réessayer
    }
  }
}

/*/
void reed_task(void *pvParameter){
    while(1){
        if(digitalRead(REED_PIN) == LOW){
            printf("Switch is open\n");
        }
        else{
            printf("Switch is closed\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
/*/

/*/
void Servo_task(void *pvParameter){
    while(true){
        if((CodeRecu == "pasOK") && (porteForce == 0)&& lastEtatPorte == 0){
              servo.write(95);
              Serial.println("Flag pasOK");
              docEtatPorte["etatPorte"] = "Porte fermée";
              serializeJson(docEtatPorte, json_EtatPorte);
              mqttClient.publish("JSON_EtatPorte", json_EtatPorte);
              lastEtatPorte = 1; 
        }
        else if ((CodeRecu == "OK") || (porteForce == 1)){
              servo.write(0);
              docEtatPorte["etatPorte"] = "Porte ouverte";
              serializeJson(docEtatPorte, json_EtatPorte);
              mqttClient.publish("JSON_EtatPorte", json_EtatPorte);
              lastEtatPorte = 0;
        }
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
/*/

void Task1code( void * parameter){
  Serial.print("Task1 is running on core ");                      // Affichage d'un message dans la console
  Serial.println(xPortGetCoreID());                               // Affichage du coeur en plein execution dans la console
  const TickType_t xDelayTask2 = 1 / portTICK_PERIOD_MS ;         // Création d'un délais de 1 ms permettant de candencer la vitesse de la boucle suivante 
  for(;;){ 
    vTaskDelay( xDelayTask2 );

    //COMMUNICATION
    if (!client.connected()) {                                    // Si le client pour le MQTT en WiFi n'est pas connecté
      reconnect();                                                // On appelle la fonction qui demande une reconnexion
    }
    client.loop();
    }
    
}

void Task_DHT22code( void* parameter){
  Serial.print("Task_DHT22 is running on core ");                      // Affichage d'un message dans la console
  Serial.println(xPortGetCoreID());                               // Affichage du coeur en plein execution dans la console
  const TickType_t xDelayTask1 = 500 / portTICK_PERIOD_MS ;       // Creation d'un délai de 500ms pour cadencer la boucle suivante

  while (true) {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }  

    static char temperatureTemp[7];
    dtostrf(t, 6, 2, temperatureTemp); //Function dtostrf converts double and floating-point values into a string.
        
    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp); //Function dtostrf converts double and floating-point values into a string.

    docDHT22["temperature"] = temperatureTemp;
    docDHT22["humidite"] = humidityTemp;
    serializeJson(docDHT22, json_DHT22);
    client.publish("JSON_DHT22", json_DHT22);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }  
}

void setup() {
  dht.begin();
  servo.attach(13);
  Serial.begin(115200); // Initialisation de la communication série

   // COMMUNICATION
  client.setServer(SERVER, 1883);                                 // On se connecte au serveur MQTT
  client.setCallback(callback);                                   // On synchronise aux messages entrant en MQTT 
  setup_wifi();                                                   // Démarrage de la communication Wi-Fi

  xTaskCreatePinnedToCore(Task1code,"Task1",10000,NULL,1,&Task1,0); // création de la tache 1 sous le coeur 0
  delay(500);

  xTaskCreatePinnedToCore(Task_DHT22code,"Task2",10000,NULL,1,&Task2,1); // création de la tache 2 sous le coeur 1
  delay(500); 
}

void loop() {
}