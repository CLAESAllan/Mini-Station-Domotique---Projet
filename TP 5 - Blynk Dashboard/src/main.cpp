/*************************************************************

  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID           "TMPLh7bzSWRQ"
#define BLYNK_DEVICE_NAME           "Quickstart Device"
#define BLYNK_AUTH_TOKEN            "bd3c5oiz5Hn8gl2A_sJZhbGc0mdhhc7Z"


// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_NeoPixel.h>

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Ordi de Gilles";
char pass[] = "12345678";

#define luminosite 39
#define ledRouge 32
#define RGB 0
#define BP 12
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT  1

// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 50 // Set BRIGHTNESS to about 1/5 (max = 255)

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, RGB, NEO_GBR + NEO_KHZ800);


int value;
int valeur = 0;
int valeurMode = 0;
int Rouge = 0, Vert = 0, Bleu = 0;
int luminositeAmbiante = 0,luminositeMappe = 0,luminositeMappe255 = 0;
BlynkTimer timer;

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  valeur = !valeur;
  // Update state
  //int value = param.asInt();
  //Blynk.virtualWrite(V1, value);
}

BLYNK_WRITE(V5){
  int RougeRecu = param.asInt();
  Rouge = RougeRecu;
}
BLYNK_WRITE(V6){
  int VertRecu = param.asInt();
  Vert = VertRecu;
}
BLYNK_WRITE(V7){
  int BleuRecu = param.asInt();
  Bleu = BleuRecu;
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, millis() / 1000);
}
WidgetLED ledBleu(V4);

void setup()
{
  // Debug console
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();

  pinMode(ledRouge,OUTPUT);
  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);
}

void loop()
{
  Blynk.run();
  timer.run();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
  luminositeAmbiante = analogRead(luminosite);
  luminositeMappe= map(luminositeAmbiante, 0, 4095, 0, 100);
  Blynk.virtualWrite(V9, luminositeMappe);
  strip.fill(strip.Color(Bleu, Vert,Rouge , BRIGHTNESS));
  strip.show();
  if (valeur == 1){
      ledBleu.on();
      digitalWrite(ledRouge,HIGH);
    }
  if (valeur == 0){
      ledBleu.off(); 
      digitalWrite(ledRouge,LOW);
    }
  int etatBP = digitalRead(BP);
  if (etatBP == true){
    Blynk.virtualWrite(V1, HIGH);
    Serial.println("Allumer");
  }else{
    Blynk.virtualWrite(V1, LOW);
    Serial.println("Eteint");
  }
}
