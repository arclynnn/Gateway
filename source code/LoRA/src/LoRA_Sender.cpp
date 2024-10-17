#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>

// LoRa configuration
#define ss 5
#define rst 14
#define dio0 2

int counter = 0;

// WiFi and MQTT configuration
const char* ssid = "122";
const char* password = "123456788";
const char* mqtt_server = "mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// Function to connect to WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// MQTT callback function
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Function to reconnect to MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "cobamqtt";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("/esp32/topicdipo/out", "hello world");
      client.subscribe("/esp32/topicdipo/in");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Setup function for LoRa and MQTT
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Setup WiFi and MQTT
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Setup LoRa transceiver
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(920E6)) {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

// Loop function to send LoRa packets and MQTT messages
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Send a LoRa packet every 10 seconds
  Serial.print("Sending LoRa packet: ");
  Serial.println(counter);
  
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();
  
  // Publish the LoRa packet data to MQTT
  snprintf(msg, MSG_BUFFER_SIZE, "LoRa message #%d", counter);
  Serial.print("Publishing MQTT message: ");
  Serial.println(msg);
  client.publish("/esp32/topicdipo/out", msg);

  counter++;
  
  // Delay to control the packet sending rate
  delay(10000);
}
