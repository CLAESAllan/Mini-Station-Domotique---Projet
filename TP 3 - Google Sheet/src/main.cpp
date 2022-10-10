#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "Ordi de Gilles";
const char *password = "12345678";
String GOOGLE_SCRIPT_ID = "AKfycbzx5iPhR_IbFFEwz8NUzR9AvscwculLY23C_vb9_HkX8q5RbjNS";

const char * root_ca=\
"-----BEGIN CERTIFICATE-----\n" \
"MIIN7TCCDNWgAwIBAgIQJj7Rgvf7Iw4K092uo999iDANBgkqhkiG9w0BAQsFADBG\n" \
"MQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExM\n" \
"QzETMBEGA1UEAxMKR1RTIENBIDFDMzAeFw0yMjA5MTIwODE3MDBaFw0yMjEyMDUw\n" \
"ODE2NTlaMBcxFTATBgNVBAMMDCouZ29vZ2xlLmNvbTBZMBMGByqGSM49AgEGCCqG\n" \
"SM49AwEHA0IABOCYGChq2xG/2slS+0nGt2csiDKNE8fTbn1BZAkKIrO+3wDauTKn\n" \
"WynvUvnHxztlNt86h7YXagRdCnCysMNlt9KjggvPMIILyzAOBgNVHQ8BAf8EBAMC\n" \
"B4AwEwYDVR0lBAwwCgYIKwYBBQUHAwEwDAYDVR0TAQH/BAIwADAdBgNVHQ4EFgQU\n" \
"XoTxICEfJtHnENW2boGeagAOmEowHwYDVR0jBBgwFoAUinR/r4XN7pXNPZzQ4kYU\n" \
"83E1HScwagYIKwYBBQUHAQEEXjBcMCcGCCsGAQUFBzABhhtodHRwOi8vb2NzcC5w\n" \
"a2kuZ29vZy9ndHMxYzMwMQYIKwYBBQUHMAKGJWh0dHA6Ly9wa2kuZ29vZy9yZXBv\n" \
"L2NlcnRzL2d0czFjMy5kZXIwggl/BgNVHREEggl2MIIJcoIMKi5nb29nbGUuY29t\n" \
"ghYqLmFwcGVuZ2luZS5nb29nbGUuY29tggkqLmJkbi5kZXaCFSoub3JpZ2luLXRl\n" \
"c3QuYmRuLmRldoISKi5jbG91ZC5nb29nbGUuY29tghgqLmNyb3dkc291cmNlLmdv\n" \
"b2dsZS5jb22CGCouZGF0YWNvbXB1dGUuZ29vZ2xlLmNvbYILKi5nb29nbGUuY2GC\n" \
"CyouZ29vZ2xlLmNsgg4qLmdvb2dsZS5jby5pboIOKi5nb29nbGUuY28uanCCDiou\n" \
"Z29vZ2xlLmNvLnVrgg8qLmdvb2dsZS5jb20uYXKCDyouZ29vZ2xlLmNvbS5hdYIP\n" \
"Ki5nb29nbGUuY29tLmJygg8qLmdvb2dsZS5jb20uY2+CDyouZ29vZ2xlLmNvbS5t\n" \
"eIIPKi5nb29nbGUuY29tLnRygg8qLmdvb2dsZS5jb20udm6CCyouZ29vZ2xlLmRl\n" \
"ggsqLmdvb2dsZS5lc4ILKi5nb29nbGUuZnKCCyouZ29vZ2xlLmh1ggsqLmdvb2ds\n" \
"ZS5pdIILKi5nb29nbGUubmyCCyouZ29vZ2xlLnBsggsqLmdvb2dsZS5wdIISKi5n\n" \
"b29nbGVhZGFwaXMuY29tgg8qLmdvb2dsZWFwaXMuY26CESouZ29vZ2xldmlkZW8u\n" \
"Y29tggwqLmdzdGF0aWMuY26CECouZ3N0YXRpYy1jbi5jb22CD2dvb2dsZWNuYXBw\n" \
"cy5jboIRKi5nb29nbGVjbmFwcHMuY26CEWdvb2dsZWFwcHMtY24uY29tghMqLmdv\n" \
"b2dsZWFwcHMtY24uY29tggxna2VjbmFwcHMuY26CDiouZ2tlY25hcHBzLmNughJn\n" \
"b29nbGVkb3dubG9hZHMuY26CFCouZ29vZ2xlZG93bmxvYWRzLmNughByZWNhcHRj\n" \
"aGEubmV0LmNughIqLnJlY2FwdGNoYS5uZXQuY26CEHJlY2FwdGNoYS1jbi5uZXSC\n" \
"EioucmVjYXB0Y2hhLWNuLm5ldIILd2lkZXZpbmUuY26CDSoud2lkZXZpbmUuY26C\n" \
"EWFtcHByb2plY3Qub3JnLmNughMqLmFtcHByb2plY3Qub3JnLmNughFhbXBwcm9q\n" \
"ZWN0Lm5ldC5jboITKi5hbXBwcm9qZWN0Lm5ldC5jboIXZ29vZ2xlLWFuYWx5dGlj\n" \
"cy1jbi5jb22CGSouZ29vZ2xlLWFuYWx5dGljcy1jbi5jb22CF2dvb2dsZWFkc2Vy\n" \
"dmljZXMtY24uY29tghkqLmdvb2dsZWFkc2VydmljZXMtY24uY29tghFnb29nbGV2\n" \
"YWRzLWNuLmNvbYITKi5nb29nbGV2YWRzLWNuLmNvbYIRZ29vZ2xlYXBpcy1jbi5j\n" \
"b22CEyouZ29vZ2xlYXBpcy1jbi5jb22CFWdvb2dsZW9wdGltaXplLWNuLmNvbYIX\n" \
"Ki5nb29nbGVvcHRpbWl6ZS1jbi5jb22CEmRvdWJsZWNsaWNrLWNuLm5ldIIUKi5k\n" \
"b3VibGVjbGljay1jbi5uZXSCGCouZmxzLmRvdWJsZWNsaWNrLWNuLm5ldIIWKi5n\n" \
"LmRvdWJsZWNsaWNrLWNuLm5ldIIOZG91YmxlY2xpY2suY26CECouZG91YmxlY2xp\n" \
"Y2suY26CFCouZmxzLmRvdWJsZWNsaWNrLmNughIqLmcuZG91YmxlY2xpY2suY26C\n" \
"EWRhcnRzZWFyY2gtY24ubmV0ghMqLmRhcnRzZWFyY2gtY24ubmV0gh1nb29nbGV0\n" \
"cmF2ZWxhZHNlcnZpY2VzLWNuLmNvbYIfKi5nb29nbGV0cmF2ZWxhZHNlcnZpY2Vz\n" \
"LWNuLmNvbYIYZ29vZ2xldGFnc2VydmljZXMtY24uY29tghoqLmdvb2dsZXRhZ3Nl\n" \
"cnZpY2VzLWNuLmNvbYIXZ29vZ2xldGFnbWFuYWdlci1jbi5jb22CGSouZ29vZ2xl\n" \
"dGFnbWFuYWdlci1jbi5jb22CGGdvb2dsZXN5bmRpY2F0aW9uLWNuLmNvbYIaKi5n\n" \
"b29nbGVzeW5kaWNhdGlvbi1jbi5jb22CJCouc2FmZWZyYW1lLmdvb2dsZXN5bmRp\n" \
"Y2F0aW9uLWNuLmNvbYIWYXBwLW1lYXN1cmVtZW50LWNuLmNvbYIYKi5hcHAtbWVh\n" \
"c3VyZW1lbnQtY24uY29tggtndnQxLWNuLmNvbYINKi5ndnQxLWNuLmNvbYILZ3Z0\n" \
"Mi1jbi5jb22CDSouZ3Z0Mi1jbi5jb22CCzJtZG4tY24ubmV0gg0qLjJtZG4tY24u\n" \
"bmV0ghRnb29nbGVmbGlnaHRzLWNuLm5ldIIWKi5nb29nbGVmbGlnaHRzLWNuLm5l\n" \
"dIIMYWRtb2ItY24uY29tgg4qLmFkbW9iLWNuLmNvbYINKi5nc3RhdGljLmNvbYIU\n" \
"Ki5tZXRyaWMuZ3N0YXRpYy5jb22CCiouZ3Z0MS5jb22CESouZ2NwY2RuLmd2dDEu\n" \
"Y29tggoqLmd2dDIuY29tgg4qLmdjcC5ndnQyLmNvbYIQKi51cmwuZ29vZ2xlLmNv\n" \
"bYIWKi55b3V0dWJlLW5vY29va2llLmNvbYILKi55dGltZy5jb22CC2FuZHJvaWQu\n" \
"Y29tgg0qLmFuZHJvaWQuY29tghMqLmZsYXNoLmFuZHJvaWQuY29tggRnLmNuggYq\n" \
"LmcuY26CBGcuY2+CBiouZy5jb4IGZ29vLmdsggp3d3cuZ29vLmdsghRnb29nbGUt\n" \
"YW5hbHl0aWNzLmNvbYIWKi5nb29nbGUtYW5hbHl0aWNzLmNvbYIKZ29vZ2xlLmNv\n" \
"bYISZ29vZ2xlY29tbWVyY2UuY29tghQqLmdvb2dsZWNvbW1lcmNlLmNvbYIIZ2dw\n" \
"aHQuY26CCiouZ2dwaHQuY26CCnVyY2hpbi5jb22CDCoudXJjaGluLmNvbYIIeW91\n" \
"dHUuYmWCC3lvdXR1YmUuY29tgg0qLnlvdXR1YmUuY29tghR5b3V0dWJlZWR1Y2F0\n" \
"aW9uLmNvbYIWKi55b3V0dWJlZWR1Y2F0aW9uLmNvbYIPeW91dHViZWtpZHMuY29t\n" \
"ghEqLnlvdXR1YmVraWRzLmNvbYIFeXQuYmWCByoueXQuYmWCGmFuZHJvaWQuY2xp\n" \
"ZW50cy5nb29nbGUuY29tghtkZXZlbG9wZXIuYW5kcm9pZC5nb29nbGUuY26CHGRl\n" \
"dmVsb3BlcnMuYW5kcm9pZC5nb29nbGUuY26CGHNvdXJjZS5hbmRyb2lkLmdvb2ds\n" \
"ZS5jbjAhBgNVHSAEGjAYMAgGBmeBDAECATAMBgorBgEEAdZ5AgUDMDwGA1UdHwQ1\n" \
"MDMwMaAvoC2GK2h0dHA6Ly9jcmxzLnBraS5nb29nL2d0czFjMy9RT3ZKME4xc1Qy\n" \
"QS5jcmwwggEEBgorBgEEAdZ5AgQCBIH1BIHyAPAAdgBRo7D1/QF5nFZtuDd4jwyk\n" \
"eswbJ8v3nohCmg3+1IsF5QAAAYMw/O3FAAAEAwBHMEUCIEQk2EUAX3M5/z1Q28Ji\n" \
"uu2OPoUsKBugb++/BfvvwyZqAiEAri32JynGOZ3mehit+ypuFsqSAL/lS1VJKN7v\n" \
"BOHV9YUAdgBGpVXrdfqRIDC1oolp9PN9ESxBdL79SbiFq/L8cP5tRwAAAYMw/O3Y\n" \
"AAAEAwBHMEUCIQCKeeZZqKMbL0M0daBbhisQlttAo3sF4FJ0wMtWPCWg2AIgI7cf\n" \
"q6XI3GWrxRy6A2nMGw4MYwPiieUBrxaLbOTA7J8wDQYJKoZIhvcNAQELBQADggEB\n" \
"AEdWIZMhoQKC7DxMgoG7IIO8QeMFNsEndd7Y34k+B0XIxmSniGQWeDW42Af8yLUW\n" \
"ejJCh11CgpEsBSUFR47SFkW2JDAjGUc98iQM3q0+Zsw4g2LBWtEGgIqfaAGVHNsu\n" \
"OSSFNdH28mi4oo7Kw6c2NK7JkyKVt3nhZOCyduwdvegAjG5hOM+Ydwz2IEjZMxbb\n" \
"4hHSQVrzPZErLsrIIXRAwnouLDrPfSJKZmZvBbwYLUcRUaOrRHaaRI7HBBxbUC+b\n" \
"oDqQ3OBA2PGdABMBs/7ZbfBD2w0PX+Zqief4mvvldfmJJyWzSAj6rIA4qMkfMv76\n" \
"0csBvOhYj2iDFJCsyJWTUhc=\n" \
"-----END CERTIFICATE-----\n";


WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
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
String strTemp, strHum, strParameter;
void loop() {
  //Send an HTTP POST request every delay
  if ((millis() - lastTime) > timerDelay) {
    strTemp = String(random(100), DEC);
    strHum = String(random(100), DEC);
    Serial.println(strTemp);
    Serial.println(strHum);
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      strParameter = "temperature=" + strTemp + "&humidity=" + strHum;
      sendData(strParameter);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

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