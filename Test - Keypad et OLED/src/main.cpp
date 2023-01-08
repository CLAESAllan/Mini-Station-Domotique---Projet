// Bibliothèques
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <ESP32Tone.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Identifiant au WiFi
const char* ssid = "Ordi de Gilles";
const char* password = "12345678";

// IP du broker MQTT
const char* mqtt_server = "192.168.137.1";  //192.168.137.1   172.18.0.3

// Initialisation du client ESP
WiFiClient espClient;
PubSubClient client(espClient);
String clientId = "ello";

// Caractéristique du OLED 
#define SCREEN_WIDTH 128 // Largeur du OLED en pixel
#define SCREEN_HEIGHT 64 // Hauteur du OLED en pixel

// Caractéristique du clavier matriciel
#define ROW_NUM     4  // Quatre rangée
#define COLUMN_NUM  4  // Quatre colonne  

// Définition des pins aux matériels
#define LEDrouge 26
#define LEDvert 25
#define BUZZZER_PIN 32
byte pin_rows[ROW_NUM] = {15, 2, 0, 4};    
byte pin_column[COLUMN_NUM] = {16, 17, 5, 18}; 


// Documents JSON
char json_codeBON[256];
char json_resetInfraction[256];
DynamicJsonDocument doccodeBON(256);
DynamicJsonDocument docresetInfraction(256);

// Définition du tableau pour le clavier matriciel
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Déclaration du clavier matriciel 
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

// Déclaration du SSD1306 (OLED) connectée via I2C (SDA, SCL)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Variables
int codeROW = 40;                                                             // Place du chiffre introduit sur le OLED
volatile u_int numberOfEntry = 0;                                             // Nombre de chiffre introduit
int boucleBuzz = 1000;                                                        // Nombre de passage dans la boucle du buzzer
bool chiffre_valid;                                                           // Vérification qu'un chiffre est introduit 
bool check_validation = true;                                                 // Vérification que le code introduit est bon
char chiffres[10]= {'0','1','2', '3', '4', '5', '6','7', '8', '9'};           // Tableau des chiffres disponibles
char motDePasse[4] = {'3','6','1','0'};                                       // Mot de passe qui est correcte
char codeIntroduit[4] = {};                                                   // Tableau qui retient le code introduit
int temperature;                                                              // Variable de la température
int humidite;                                                                 // Variable de l'humidité
int newDataTemp;                                                              // Nouvelle variable de la température
int newDataHum;                                                               // Nouvelle variable de l'humidité

void setup_wifi() {          // Fonction de connection au WiFi
  delay(10);
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


void callback(String topic, byte* message, unsigned int length) {         // Fonction de 'callback' pour recevoir les données de NodeRED
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

 for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
 }
 if (topic == "temperatureOUT"){
  temperature = messageTemp.toInt();
  newDataTemp = 1;
  
 }
 if (topic == "humiditeOUT"){
  humidite = messageTemp.toInt();
  newDataHum = 1;
 }
 
 Serial.println();
}

void reconnect() {          // Fonction de reconnexion
 // Boucle jusqu'à la reconnexion
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Tentative de reconnexion 
 
 if (client.connect(clientId.c_str(),"","095f3cdd2282")){
  Serial.println("connected");
  client.subscribe("codeBON");                   //
  client.subscribe("ActionVerrou");              //
  client.subscribe("temperatureOUT");            // Topic de réception de la température
  client.subscribe("humiditeOUT");               // Topic de réception de l'humidité
  client.subscribe("resetInfraction");           //
 } 
 else{
  Serial.print("failed, rc=");
  Serial.print(client.state());
  Serial.println(" try again in 5 seconds");
  // Attendre 5s avant de réessayer 
  delay(5000);
  }
 }
}

char compareArray(char a[],char b[],int size)	{       //Fonction de comparaison
	int i;
	for(i=0;i<size;i++){
		if(a[i]!=b[i])
			return 1;
	}
	return 0;
}

void AffichageBase(){       // Fonction d'affichage du menu principal       
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(13, 12);
    display.println("Introduire code :");
    display.drawRoundRect(0, 0, 128, 64, 8, WHITE);
    display.drawRect(35,28,56,23,WHITE);
    display.display();
}

void AffichageDHT22(){      // Fonction d'affichage des données du DHT22
  display.setTextSize(1);
  display.drawRoundRect(0, 0, 128, 64, 8, WHITE);
  display.drawRoundRect(30,26,60,31,8,WHITE);
  // Logo de la goute
  display.drawPixel(39,43,WHITE);
  display.drawPixel(39,44,WHITE);

  display.drawPixel(38,45,WHITE);
  display.drawPixel(38,46,WHITE);

  display.drawPixel(40,45,WHITE);
  display.drawPixel(40,46,WHITE);

  display.drawPixel(37,47,WHITE);
  display.drawPixel(37,48,WHITE);

  display.drawPixel(41,47,WHITE);
  display.drawPixel(41,48,WHITE);

  display.drawPixel(36,49,WHITE);
  display.drawPixel(36,50,WHITE);
  display.drawPixel(36,51,WHITE);

  display.drawPixel(42,49,WHITE);
  display.drawPixel(42,50,WHITE);
  display.drawPixel(42,51,WHITE);

  display.drawPixel(37,52,WHITE);

  display.drawPixel(41,52,WHITE);

  display.drawPixel(38,53,WHITE);
  display.drawPixel(39,53,WHITE);
  display.drawPixel(40,53,WHITE);

  //  Logo du thermomètre
  display.drawLine(37,35,37,37,WHITE);
  display.drawLine(38,29,38,38,WHITE);
  display.drawLine(39,32,39,38,WHITE);
  display.drawLine(40,29,40,38,WHITE);
  display.drawLine(41,35,41,37,WHITE);
  display.drawPixel(39,28,WHITE);

  // Emplacement des données
  display.setCursor(47, 30);
  display.println("T:");
  display.setCursor(72, 30);
  display.cp437(true);
  display.write(248);
  display.setCursor(79, 30);
  display.println("C");
  display.setCursor(47, 45);
  display.println("H:");
  display.setCursor(77, 45);
  display.println("%");
  display.setTextSize(1);
  display.setCursor(58, 30);
  display.println(temperature);
  display.setCursor(58, 45);
  display.println(humidite);
  display.display(); 
}

void setup() {              // Fonction principale
  Serial.begin(115200);                   // Initialisation du port série
  setup_wifi();                           // Appel de la fonction pour connecter au WiFi
  client.setServer(mqtt_server, 1883);    // Initialisation au serveur MQTT au port 1883
  client.setCallback(callback);           // Initialisation de la fonction de callback au client
  
  // Configuration du comportement des PIN
  pinMode(LEDvert,OUTPUT);
  pinMode(LEDrouge,OUTPUT);
  pinMode(BUZZZER_PIN,OUTPUT);

  // Vérification de la connectivité du OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(500);
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  AffichageBase();     // Appel de la fonction pour afficher l'affichage du menu principale
}

void loop() {               // Boucle principale   
  if (!client.connected()) {
    reconnect();
    }
  if(!client.loop()){
    client.connect(clientId.c_str(),"","095f3cdd2282");
    delay(100);
    }
  char key = keypad.getKey();
  chiffre_valid = false;
 
  if (key == '#' ){ 
    check_validation = false;
    if(compareArray(motDePasse,codeIntroduit,4) == 0){
      digitalWrite(LEDvert,HIGH);
      display.clearDisplay();
      display.setTextSize(1);
      display.drawRect(25,7,81,14,WHITE);
      display.setCursor(30 , 10);
      display.println("Code correct");
      display.setTextColor(WHITE);
      display.display();
      AffichageDHT22();
      doccodeBON["etatCode"] = "OK";
      serializeJson(doccodeBON, json_codeBON);
      client.publish("JSON_codeBON", json_codeBON);
      Serial.println(json_codeBON);
      while (boucleBuzz <4000){
        tone(BUZZZER_PIN,boucleBuzz,125);
        boucleBuzz+=500;
         }
      boucleBuzz = 1000;        
         }
    else{
      tone(BUZZZER_PIN,2000,500);
      digitalWrite(LEDrouge,HIGH);
      display.clearDisplay();
      display.drawRoundRect(0, 0, 128, 64, 8, WHITE);
      display.drawCircle(64,35,13,WHITE);
      display.drawLine(55,44,73,26,WHITE);
      display.setTextSize(1);
      display.setCursor(21, 10);
      display.println("Code incorrect");
      doccodeBON["etatCode"] = "pasOK";
      serializeJson(doccodeBON, json_codeBON);
      client.publish("JSON_codeBON", json_codeBON);
      display.display();
       }
   }

  if (key == 'A' && compareArray(motDePasse,codeIntroduit,4) == 0 ){          
    docresetInfraction["etatReset"] = "Reset" ;
    serializeJson(docresetInfraction, json_resetInfraction);
    client.publish("JSON_resetInfraction", json_resetInfraction);
   }

  if(key == 'D' ){
    codeIntroduit[0] = '0';
    codeIntroduit[1] = '0';
    codeIntroduit[2] = '0';
    codeIntroduit[3] = '0';
    AffichageBase();
    numberOfEntry = 0;
    codeROW = 40;
    digitalWrite(LEDrouge,LOW);
    digitalWrite(LEDvert,LOW);
    
    doccodeBON["etatCode"] = "pasOK";
    serializeJson(doccodeBON, json_codeBON);
    client.publish("JSON_codeBON", json_codeBON);

    docresetInfraction["etatReset"] = "pasReset";
    serializeJson(docresetInfraction, json_resetInfraction);
    client.publish("JSON_resetInfraction", json_resetInfraction);
    check_validation = true;
    }

  for (int x = 0; x < 10; x++){
    if (key == chiffres[x]){
     chiffre_valid = true;
     break;
     }
   }

  if (key && chiffre_valid && (numberOfEntry <= 3) && check_validation) {
    tone(BUZZZER_PIN,rand()%7500 + 2500,175);
    codeIntroduit[numberOfEntry] = key;
    display.setTextSize(2);
    display.setCursor(codeROW, 33);
    display.println(key);
    for (int passageVerfi = 0; passageVerfi < 4; passageVerfi++){
      if(key == motDePasse[passageVerfi] && numberOfEntry == passageVerfi){
        codeIntroduit[passageVerfi] = key;
        Serial.println(key);
        break;
      }
    }
    display.display();
    codeROW += 12;
    numberOfEntry++;
  }
}  