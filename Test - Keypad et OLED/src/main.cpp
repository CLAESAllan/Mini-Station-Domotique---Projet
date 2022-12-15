#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <ESP32Tone.h>

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

int codeROW = 33;
volatile u_int numberOfEntry = 0;
int boucleBuzz = 1000;
bool chiffre_valid;
bool check_validation = true;
char chiffres[10]= {'0','1','2', '3', '4', '5', '6','7', '8', '9'};
char motDePasse[4] = {'3','6','1','0'};
char codeIntroduit[4] = {};

void setup() {
  Serial.begin(115200);
  pinMode(LEDvert,OUTPUT);
  pinMode(LEDrouge,OUTPUT);
  pinMode(BUZZZER_PIN,OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 8);
  // Display static text
  display.println("Introduire code :");
  display.display(); 
}

void loop() {
  char key = keypad.getKey();
  chiffre_valid = false;

  //int tailleTableau = sizeof(codeIntroduit);

if (key == '#' ){
    check_validation = false;
     if(compareArray(motDePasse,codeIntroduit,4) == 0){
        digitalWrite(LEDvert,HIGH);
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(25 , 20);
        display.println("Code correct");
        display.display();
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
        display.setTextSize(1);
        display.setCursor(25, 20);
        display.println("Code incorrect");
        display.display();
        
    }
}
  
  if(key == 'D' ){
    codeIntroduit[0] = '0';
    codeIntroduit[1] = '0';
    codeIntroduit[2] = '0';
    codeIntroduit[3] = '0';
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(10, 8);
    display.println("Introduire code :");
    display.display();
    numberOfEntry = 0;
    codeROW = 33;
    digitalWrite(LEDrouge,LOW);
    digitalWrite(LEDvert,LOW);
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
    display.setCursor(codeROW, 25);
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