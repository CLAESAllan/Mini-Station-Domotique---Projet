//Libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

//Pins used
#define DHTPIN 26
#define DHTTYPE DHT11
#define light_sensor_pin 39
#define ledRouge 32
#define BP 12

DHT dht(DHTPIN, DHTTYPE); //Declaration of the type of DHT used

int Lum;

//Allows you to connect to the wifi point
const char *ssid = "Ordi de Gilles";
const char *password = "12345678";

String GOOGLE_SCRIPT_ID = "AKfycbzx5iPhR_IbFFEwz8NUzR9AvscwculLY23C_vb9_HkX8q5RbjNS"; //Connect to the Google Sheets page

//Certificate to access the google Sheets page. Without this certificate, no communication will be made.
const char * root_ca=\
"-----BEGIN CERTIFICATE-----\n" \
"MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n" \
"A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n" \
"b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n" \
"MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n" \
"YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n" \
"aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n" \
"jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n" \
"xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n" \
"1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n" \
"snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n" \
"U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n" \
"9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n" \
"BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n" \
"AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n" \
"yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n" \
"38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n" \
"AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n" \
"DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n" \
"HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n" \
"-----END CERTIFICATE-----\n";


WiFiClientSecure client; //Client type use for WiFi communication

void setup() {
  Serial.begin(115200); //Setup the monitor serial
  delay(10);

  dht.begin(); //Initialize the module DHT11
  
  WiFi.mode(WIFI_STA); //ESP32 connects to an access point
  WiFi.begin(ssid, password); //Initializes the network settings of the and provides the current status.

  //Defines inputs / outputs
  pinMode(ledRouge,OUTPUT);
  pinMode(BP,INPUT);

  Serial.println("Started");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) { //Returns the connection status
    delay(500);                           //WL_CONNECTED : assigned when connected to a WiFi network
    Serial.print("."); //Print (".") as long as it is not connected
  }
  randomSeed(analogRead(0));
  Serial.println("TP3 GoogleSheets ready...");
}

unsigned long lastTime = 0; //We use long because the time (in ms) will quickly become a larger number than can be stored in an int.
unsigned long timerDelay = 10000; // interval at which data is sent(ms)

//Variables used for the frame
String strTemp;
String strHum;
String strParameter;
String strLum;
String strEtatBP;
String strEtatLed;

int etatLed;

//Allows you to connect and return errors when connecting to the Google Sheets page
void sendData(String params) {
  HTTPClient http;
  String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+params;
  Serial.println(url);
  Serial.println("Making a request");
  
  // Your Domain name with URL path or IP address with path
  http.begin(url, root_ca); //Specify the URL and certificate
  
  // Send HTTP GET request
  int httpCode = http.GET();
  if (httpCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpCode);
    String payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpCode);
  }
  // Free resources
  http.end();
}

void loop() {  
  if(digitalRead(BP)){ 
    //Send an HTTP POST request every delay
    if ((millis() - lastTime) > timerDelay) {
      strTemp = dht.readTemperature() ;
      strHum = dht.readHumidity();
      Lum = analogRead(light_sensor_pin) ;
      strLum = map(Lum, 0, 2700, 100, 0);
      Serial.println(strTemp);
      Serial.println(strHum);
      Serial.println(strLum);

      //Check WiFi connection status
      if(WiFi.status()== WL_CONNECTED){
        String strParameter = "temperature=" + strTemp + "&humidity=" + strHum + "&luminosity=" + strLum;
        sendData(strParameter);
        digitalWrite(ledRouge,HIGH);
        delay(100);
        digitalWrite(ledRouge,LOW); 
      }
      else {
        Serial.println("WiFi Disconnected");
      }
      lastTime = millis();
    
    }
  }
}