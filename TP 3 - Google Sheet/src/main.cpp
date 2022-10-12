#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

#define DHTPIN 26
#define DHTTYPE DHT11

#define light_sensor_pin 39

#define ledRouge 32
#define BP 12


DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "Ordi de Gilles";
const char *password = "12345678";
String GOOGLE_SCRIPT_ID = "AKfycbzx5iPhR_IbFFEwz8NUzR9AvscwculLY23C_vb9_HkX8q5RbjNS";

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


WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  delay(10);

  dht.begin();
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  pinMode(ledRouge,OUTPUT);
  pinMode(BP,INPUT);

  Serial.println("Started");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(analogRead(0));
  Serial.println("TP3 GoogleSheets ready...");
}

unsigned long lastTime = 0;
unsigned long timerDelay = 10000;
String strTemp;
String strHum;
String strParameter;
String strLum;
String strEtatBP;
String strEtatLed;
int etatLed;

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
      digitalWrite(ledRouge,HIGH);
      delay(100);
      digitalWrite(ledRouge,LOW);
  }
  //Send an HTTP POST request every delay
  if ((millis() - lastTime) > timerDelay) {
    strTemp = dht.readTemperature() ;
    strHum = dht.readHumidity();
    strLum = analogRead(light_sensor_pin) ;
    Serial.println(strTemp);
    Serial.println(strHum);
    Serial.println(strLum);

    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      String strParameter = "temperature=" + strTemp + "&humidity=" + strHum + "&luminosity=" + strLum;
      sendData(strParameter);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
