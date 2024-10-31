#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <LoRa.h>

// WiFi
const char *ssid = "dipo"; // Enter your Wi-Fi name
const char *password = "divo1234";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "lora/data/out";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// LoRa
#define ss 5
#define rst 14
#define dio0 2

int counter = 0;

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);
    
    // Initialize WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");

    // Initialize MQTT
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "client-esp32-mqtt-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    client.subscribe(topic);

    // Setup LoRa transceiver module
    LoRa.setPins(ss, rst, dio0);
    while (!LoRa.begin(920E6)) {  // Adjust frequency for your region
        Serial.println(".");
        delay(500);
    }
    LoRa.setSyncWord(0xF3);
    Serial.println("LoRa Initializing OK!");
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
}

void loop() {
    client.loop(); // Handle MQTT tasks
    
    // LoRa transmission
    Serial.print("Sending LoRa packet: ");
    Serial.println(counter);

    LoRa.beginPacket();
    LoRa.print("LoRa message ");
    LoRa.print(counter);
    LoRa.endPacket();

    // Publish the same message via MQTT
    String msg = "LoRa message " + String(counter);
    client.publish(topic, msg.c_str());

    counter++;
    delay(5000); // Send every 10 seconds
}
