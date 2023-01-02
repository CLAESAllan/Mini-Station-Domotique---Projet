#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <ESP32Tone.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>

//Change the credentials below, so your ESP32 connects to your router
const char* ssid = "Ordi de Gilles";
const char* password = "12345678";

//Connect to your MQTT broker
const char* mqtt_server = "192.168.137.1";  //192.168.137.1   172.18.0.3

//Initializes the espClient.
WiFiClient espClient;
PubSubClient client(espClient);
long now = millis();
long lastMeasure = 0;
String clientId = "ello";

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

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define ROW_NUM     4  // four rows
#define COLUMN_NUM  4  // four columns

#define LEDrouge 26
#define LEDvert 25
#define BUZZZER_PIN 32

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

//Function de comparaison
char compareArray(char a[],char b[],int size)	{
	int i;
	for(i=0;i<size;i++){
		if(a[i]!=b[i])
			return 1;
	}
	return 0;
}

byte pin_rows[ROW_NUM] = {15, 2, 0, 4};    // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte pin_column[COLUMN_NUM] = {16, 17, 5, 18}; // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int codeROW = 40;
volatile u_int numberOfEntry = 0;
int boucleBuzz = 1000;
bool chiffre_valid;
bool check_validation = true;
char chiffres[10]= {'0','1','2', '3', '4', '5', '6','7', '8', '9'};
char motDePasse[4] = {'3','6','1','0'};
char codeIntroduit[4] = {};
int temperature;
int humidite;

void callback(String topic, byte* message, unsigned int length) {
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
 }
 if (topic == "humiditeOUT"){
  humidite = messageTemp.toInt();
 }
 
 Serial.println();
}
void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 
 if (client.connect(clientId.c_str(),"","095f3cdd2282")) {
 Serial.println("connected");
 
 client.subscribe("codeBON");
 client.subscribe("ActionVerrou");
 client.subscribe("temperatureOUT");
 client.subscribe("humiditeOUT");
 client.subscribe("resetInfraction");

 } else {
 Serial.print("failed, rc=");
 Serial.print(client.state());
 Serial.println(" try again in 5 seconds");
 // Wait 5 seconds before retrying
 delay(5000);
 }
 }
}

void AffichageBase(){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(13, 12);
    display.println("Introduire code :");
    display.drawRoundRect(0, 0, 128, 64, 8, WHITE);
    display.drawRect(35,28,56,23,WHITE);
    display.display();
}

void AffichageDHT22(){
  display.setTextSize(1);
  display.drawRoundRect(0, 0, 128, 64, 8, WHITE);
  display.drawRoundRect(30,26,60,31,8,WHITE);
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

  display.drawLine(37,35,37,37,WHITE);
  display.drawLine(38,29,38,38,WHITE);
  display.drawLine(39,32,39,38,WHITE);
  display.drawLine(40,29,40,38,WHITE);
  display.drawLine(41,35,41,37,WHITE);
  display.drawPixel(39,28,WHITE);

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

void setup() {
  Serial.begin(115200);
  setup_wifi();
  // set server mqtt on port 1883 to the client
  client.setServer(mqtt_server, 1883);
  // set the function callback to the client
  client.setCallback(callback);

  pinMode(LEDvert,OUTPUT);
  pinMode(LEDrouge,OUTPUT);
  pinMode(BUZZZER_PIN,OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  AffichageBase();
}

void loop() {
  if (!client.connected()) {
    reconnect();
}
  if(!client.loop()){
    client.connect(clientId.c_str(),"","095f3cdd2282");
    now = millis();
    delay(100);
}
  char key = keypad.getKey();
  chiffre_valid = false;

  //int tailleTableau = sizeof(codeIntroduit);
 
  
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
        AffichageDHT22();
        display.display();
        client.publish("codeBON","OK");
        Serial.print(key);
        
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
        client.publish("codeBON","pasOK");
        display.display();
    }
}
  if (key == 'A' && compareArray(motDePasse,codeIntroduit,4) == 0 ){
            client.publish("resetInfraction","Reset");
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
    client.publish("codeBON","pasOK");
    client.publish("resetInfraction","pasReset");
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
     if(key == motDePasse[0] && numberOfEntry == 0){
        codeIntroduit[0] = key;
        Serial.println(key);
      }else if (key == motDePasse[1] && numberOfEntry == 1){
        codeIntroduit[1] = key;
        Serial.println(key);
      }else if (key == motDePasse[2] && numberOfEntry == 2){
        codeIntroduit[2] = key;
        Serial.println(key);
      }else if (key == motDePasse[3] && numberOfEntry == 3){
        codeIntroduit[3] = key;
        Serial.println(key);
      }
      

    display.display();
    codeROW += 12;
    numberOfEntry++;
    //Serial.println(key); // prints key to serial monitor
    //Serial.println(tailleTableau);
//|| key == motDePasse[1] || key == motDePasse[2] || key == motDePasse[3]
  }
}  