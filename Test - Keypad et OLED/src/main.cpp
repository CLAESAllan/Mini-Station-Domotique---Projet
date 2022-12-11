#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define ROW_NUM     4  // four rows
#define COLUMN_NUM  4  // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};



byte pin_rows[ROW_NUM] = {15, 2, 0, 4};    // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte pin_column[COLUMN_NUM] = {16, 17, 5, 18}; // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int codeROW = 33;
int numberOfEntry = 0;
bool chiffre_valid;
char chiffres[]= {'0','1','2', '3', '4', '5', '6','7', '8', '9'};
int motDePasse[4] = {3,6,1,0};
int codeIntroduit[] = {};

void setup() {
  Serial.begin(115200);

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
  int tailleTableau = sizeof(codeIntroduit);

if (key == '#' ){
  Serial.println(key);
    for (int w = 0; w < tailleTableau;w++){
      Serial.println("flag for");
      if(codeIntroduit[tailleTableau - 3] == '3' && codeIntroduit[tailleTableau - 2] == '6' && codeIntroduit[tailleTableau - 1] == '1' && codeIntroduit[tailleTableau] == '0'){
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(30, 8);
        display.println("Code correct");
        display.display();
    }
    else{
        Serial.println("flag else");
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(30, 8);
        display.println("Code incorrect");
        display.display();
        break;
    }
  }
}

  if(key == 'D' ){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(10, 8);
    display.println("Introduire code :");
    display.display();
    numberOfEntry = 0;
    codeROW = 33;
  }

for (int x = 0; x < 10; x++){
    if (key == chiffres[x]){
     chiffre_valid = true;
     break;
  }
}

  if (key && chiffre_valid && (numberOfEntry <= 3)) {
    codeIntroduit[numberOfEntry] = key;
    display.setTextSize(2);
    display.setCursor(codeROW, 25);
    display.println(key);

    display.setCursor(codeROW, 45);

    display.display();
    codeROW += 12;
    numberOfEntry +=1;
    Serial.println(key); // prints key to serial monitor
    Serial.println(numberOfEntry);
  }
}
  