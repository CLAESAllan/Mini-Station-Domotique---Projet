#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Replace these with your WiFi credentials.
const char* ssid = "Ordi de Gilles";
const char* password = "12345678";

// Replace this with the IP address of your MQTT broker.
const char* mqttServer = "192.168.137.1";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  vTaskDelay(100 / portTICK_PERIOD_MS);
  // Connect to WiFi.
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  // Connect to MQTT broker.
  while (!client.connect("ESP32Client")) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
  }
}

void mqttTask(void* parameter){
  setup_wifi();

  // Main loop.
  while (true) {
    if (!client.connected()) {
      reconnect();
    }
    // Check for incoming messages.
    client.loop();

    // Wait a bit before checking again.
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void callback(char* topic, byte* payload, unsigned int length){
  // Print the message to the console.
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  // Subscribe to a topic
  //client.subscribe("ESP32/message", "Envoie message");
}

void setup(){
  // Initialize serial and start the FreeRTOS task.
  Serial.begin(115200);
  xTaskCreate(mqttTask, "MQTTTask", 4096, NULL, 1, NULL);
  vTaskStartScheduler();
}

void loop(){
  // Do nothing. Everything is done in the FreeRTOS task.
}
