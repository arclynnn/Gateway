#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define ss 5
#define rst 14
#define dio0 2

const char *ssid = "dipo"; // Wi-Fi name
const char *password = "divo1234";  // Wi-Fi password

const char *mqtt_server = "broker.emqx.io";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;
const char *topic = "lora/data/out";  // Changed to match sender's topic

WiFiClient espClient;
PubSubClient client(espClient);

void connectWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String client_id = "client-esp32-mqtt-";
    client_id += String(WiFi.macAddress());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected to MQTT broker.");
      client.subscribe(topic);
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  connectWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(920E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa initialized.");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  int packetSize = LoRa.parsePacket();
  Serial.print("Packet Size: ");
  Serial.println(packetSize); // Debugging line

  if (packetSize) {
    String LoRaData = "";
    while (LoRa.available()) {
      LoRaData += (char)LoRa.read();
    }

    if (LoRaData.length() > 0) {
      Serial.println("Received LoRa Data: " + LoRaData);
      Serial.print("RSSI: ");
      Serial.println(LoRa.packetRssi());
      client.publish(topic, LoRaData.c_str());
      Serial.println("Data published to MQTT broker");
    } else {
      Serial.println("Empty LoRa packet received");
    }
  } else {
    Serial.println("No LoRa packet detected");
  }

  delay(5000);
}
