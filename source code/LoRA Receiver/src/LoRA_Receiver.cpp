#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <LoRa.h>

// WiFi
const char *ssid = "dipo"; // Nama WiFi
const char *password = "divo1234";  // Password WiFi

// MQTT Broker
const char *mqtt_broker = "192.168.130.32";
const char *topic = "lora/data1"; // Topik untuk data masuk
const char *mqtt_username = "AdminMQTT";
const char *mqtt_password = "pwd123";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 26

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to Wi-Fi");

  // Initialize MQTT
  client.setServer(mqtt_broker, mqtt_port);
  while (!client.connected()) {
    String client_id = "receiver-mqtt-client-";
    client_id += String(WiFi.macAddress());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT Broker!");
    } else {
      Serial.print("Failed to connect to MQTT Broker. State: ");
      Serial.println(client.state());
      delay(2000);
    }
  }

  // Setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(920E6)) {
    Serial.println("LoRa initialization failed. Check your connections.");
    while (true); // Stop execution
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  client.loop(); // MQTT tasks
  
  // Parse incoming LoRa packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }

    if (receivedData.length() > 0) {
      Serial.print("Received data: ");
      Serial.println(receivedData);

      // Publish received data to MQTT
      if (client.publish(topic, receivedData.c_str())) {
        Serial.println("Data published to MQTT!");
      } else {
        Serial.println("Failed to publish data to MQTT.");
      }
    }

    // Print RSSI
    Serial.print("RSSI: ");
    Serial.println(LoRa.packetRssi());
  }
}
