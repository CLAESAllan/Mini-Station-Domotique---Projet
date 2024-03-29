//  Librairies
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP32Servo.h>

//  Définition des variables
#define DHTPin 4
#define DHTTYPE DHT22 //  Type de capteur
#define BP 12
#define LED 32

//  Définition des variables en int
int pinREED = 19;
int lastCheck = 0;
int lastEtatPorte = 0;
int lastEtatForce = 0;
int porteForce;
int etatReset;
int CodeRecu;

//  Définition des variables en float
float lastTemp;
float lastHum;
float newTemp;
float newHum;

//  Définition des variables en char
char json_DHT22[256];
char json_Reed[256];
char json_EtatPorte[256];

//  Permet de se connecter au point d'accès
const char* ssid = "Ordi de Gilles";  //  SSID du réseau Wi-Fi auquel nous devons nous connecter
const char* password = "12345678";  //  Mot de passe du réseau Wi-Fi

//  Connection au serveur MQTT
const char* mqtt_server = "192.168.137.1";  //  Adresse IP du server MQTT

//  Initialisations de l'espClient.
WiFiClient espClient; //  Création de "espClient" propre au client Wi-Fi
PubSubClient client(espClient);  // Création d'un système de messagerie MQTT du client

Servo myservo;  //  Création d'un objet servo pour contrôler le servo

//  Initialisation du capteur DHT.
DHT dht(DHTPin, DHTTYPE);
//  Variables auxiliaires des minuteurs
long now = millis();
long lastMeasure = 0;
String clientId = "Hello";

//  Cette fonction permet de connecter votre ESP32 à votre routeur.
void setup_wifi() {
  delay(10);  //Délai de 10ms
  //  Nous commençons par nous connecter à notre réseau WiFi
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

/*Cette fonction est exécutée lorsqu'un appareil envoie 
 *un message à un sujet auquel notre ESP32 est abonné.
 *Nous modifions la fonction ci-dessous pour ajouter une logique de programme, 
 *de sorte que lorsque le dispositif envoie un message à un sujet auquel votre ESP32 est abonné.
*/

//  Fonction de Callback quand un message MQTT arrive sur un topic (subscribe).
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);  // On affiche de quel topic il s'agit
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {  //  Nousn relions tous les caractères reçus sur le topic.
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];  //Nous enregistrons tous les caractères que nous avons besoin (uniquement).
  }
  Serial.println();
  if(topic == "JSON_EtatCode"){ //  Nous vérifions si c'est le bon topic.
    CodeRecu = messageTemp.toInt(); //  Nous transformons un string en int.
  }
  if(topic == "JSON_resetInfractionOUT"){ //  Nous vérifions si c'est le bon topic.
    etatReset = messageTemp.toInt();  //  Nous transformons un string en int.
  }
  if (topic == "BpManuelPorte"){  //  Nous vérifions si c'est le bon topic.
    porteForce = messageTemp.toInt(); //  Nous transformons un string en int.
  }
}

/* Cette fonction reconnecte votre ESP32
* à votre courtier MQTT */
void reconnect() {  
  // Boucle jusqu'à ce que nous soyons reconnectés
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    //  Tentative de connexion
 
    if (client.connect(clientId.c_str(),"","095f3cdd2282")) {
      Serial.println("connected");  //  Nous nous abonnons à ce topic.
      client.subscribe("etatREED"); //  Nous nous abonnons à ce topic.
      client.subscribe("JSON_resetInfractionOUT");  //  Nous nous abonnons à ce topic.
      client.subscribe("JSON_EtatCode");  //  Nous nous abonnons à ce topic.
      client.subscribe("BpManuelPorte");  //  Nous nous abonnons à ce topic.
    } else {  // Si la connexion rate
      Serial.print("failed, rc=");  //  Nous affichons qu'il y a une erreur.
      Serial.print(client.state());  // Nous affichons le numéro de l'erreur.
      Serial.println(" try again in 5 seconds");   // Nous affichons que nous réessayons.
      //  Nous attendons 5 secondes avant de réessayer.
      delay(5000);  
    }
  }
}

/* Cette fonction permet d'envoyer que quand nous avons un différence de 0,5°C, ce qui nous permet
d'envoyer de façon moins importante*/
int checkDHT22(){
  if((newHum < lastHum - 0.5 || newHum > lastHum + 0.5) || (newTemp < lastTemp - 0.5 || newTemp > lastTemp + 0.5)){
    lastHum = newHum;
    lastTemp = newTemp;
    return 1;
  }
  else{
    return 0;
  } 
}

void setup() {
  //  Initialisation des broches utilisées
  pinMode(LED, OUTPUT);
  pinMode(BP, INPUT);

  myservo.attach(13);  // Nous attachons le servo sur la broche 13.
 
  dht.begin();  //  Module DHT22

  Serial.begin(115200); //Initialisation du moniteur série.
  setup_wifi();
  // Définis le serveur mqtt sur le port 1883 pour le client.
  client.setServer(mqtt_server, 1883);
  // Définis la fonction callback pour le client.
  client.setCallback(callback);
  lastHum = dht.readHumidity(); // Lis l'humidité en %.
  lastTemp = dht.readTemperature(); //Lis la température en Celsius.
}

void loop() {
  StaticJsonDocument<256> docDHT22; //  Création de nos documents JSON avec une capacité de 256 octets.
  StaticJsonDocument<256> docREED;  //  Création de nos documents JSON avec une capacité de 256 octets.
  StaticJsonDocument<256> docEtatPorte; //  Création de nos documents JSON avec une capacité de 256 octets.
  
  int etatReed = digitalRead(pinREED);  //Lecture du capteur Reed.

  if (!client.connected()) {  // Si le client pour le MQTT en WiFi n'est pas connecté
    reconnect();  //  Nous appellons la fonction qui demande une reconnexion
  }

  if(!client.loop())
    client.connect(clientId.c_str(),"","095f3cdd2282");
    now = millis(); 
    delay(100);
  
  /*  Nous regardons si le capteur Reed capte ou non et si c'est le cas
  alos nous envoyons sur le topic adéquat.*/
  if(etatReed == LOW && lastCheck == 0 && etatReset == 0){
    client.publish("etatREED","La porte est sécurisé"); //  Publication sur le topic "etatREED" avec comme message "La porte est sécurisé".
    docREED["etatReed"] = 1;
    serializeJson(docREED,json_Reed); //  Nous transformons nos données en format JSON.
    client.publish("JSON_REED", json_Reed);
    lastCheck = 1;
  }else if (etatReed == HIGH && CodeRecu == 0 && lastCheck == 1){
    etatReset = 1;
    client.publish("etatREED","!!! INFRACTION !!!");
    docREED["etatReed"] = 0;
    serializeJson(docREED,json_Reed); //  Nous transformons nos données en format JSON.
    client.publish("JSON_REED", json_Reed);
    lastCheck = 0 ;
  }

  if(porteForce == 1){  //  Bouton de foraçage de Blynk.
    if (lastEtatForce == 1){
      goto BYPASSporteforce;  //Si nous rentrons dans ce if alors nous le ByPassons.
    }
    myservo.write(0); //  Position du servo.
    docEtatPorte["etatPorte"] = "Porte ouverte";
    serializeJson(docEtatPorte, json_EtatPorte);
    client.publish("JSON_EtatPorte", json_EtatPorte);
    lastEtatForce = 1;
    lastEtatPorte = 0;
    CodeRecu = 0;
  }else{
    if((CodeRecu == 0 && lastEtatPorte == 0)){
      myservo.write(95);  //  Position du servo.
      docEtatPorte["etatPorte"] = "Porte fermée";
      serializeJson(docEtatPorte, json_EtatPorte);
      client.publish("JSON_EtatPorte", json_EtatPorte);
      lastEtatPorte = 1; 
      lastEtatForce = 0;
    }
    else if ((CodeRecu == 1 && lastEtatPorte == 1)){
      myservo.write(0);
      docEtatPorte["etatPorte"] = "Porte ouverte";
      serializeJson(docEtatPorte, json_EtatPorte);
      client.publish("JSON_EtatPorte", json_EtatPorte);
      lastEtatPorte = 0;
      lastEtatForce = 0;
    }
  }
  BYPASSporteforce:
  
  //Publie les nouvelles température et humidité toutes les 3 secondes.
  if (now - lastMeasure > 3000) {
    lastMeasure = now;
 
  float h = dht.readHumidity(); //Lis l'humidité en %.
  float t = dht.readTemperature(); //Lis la température en Celsius.

  newHum = h;
  newTemp = t;

  //Vérifie si une lecture a échoué et quitte prématurément (pour réessayer).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
  return;
  }

  if (checkDHT22()){  //  Si la température est différente alors nous la transformons en format JSON.

    static char temperatureTemp[7];
    dtostrf(t, 6, 2, temperatureTemp); //La fonction dtostrf convertit les valeurs doubles et flottantes en une chaîne de caractères.

    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp); //La fonction dtostrf convertit les valeurs doubles et flottantes en une chaîne de caractères.

    docDHT22["temperature"] = temperatureTemp;  //  Insérons les valeurs de temperatureTemp dans le document.
    docDHT22["humidite"] = humidityTemp;  //  Insérons les valeurs de humidityTemp dans le document.

    // Génére le JSON minifié et l'envoie au port série.
    serializeJson(docDHT22, json_DHT22);
    client.publish("JSON_DHT22",json_DHT22);
    }
  }
}