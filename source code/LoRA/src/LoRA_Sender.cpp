#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

// Variable untuk data dummy
float temperature = 25.0; // Suhu awal (derajat Celsius)
float humidity = 50.0;    // Kelembapan awal (%)

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Sender");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //868E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(920E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  // Update dummy data
  temperature += random(-10, 10) / 10.0; // Simulasi perubahan suhu
  humidity += random(-5, 5) / 10.0;      // Simulasi perubahan kelembapan
  
  // Batasan data
  if (temperature < 15.0) temperature = 15.0; // Minimum suhu
  if (temperature > 35.0) temperature = 35.0; // Maksimum suhu
  if (humidity < 30.0) humidity = 30.0;       // Minimum kelembapan
  if (humidity > 90.0) humidity = 90.0;       // Maksimum kelembapan

  // Kirim data
  Serial.print("Sending packet: Temperature = ");
  Serial.print(temperature);
  Serial.print("°C, Humidity = ");
  Serial.print(humidity);
  Serial.println("%");

  LoRa.beginPacket();
  LoRa.print("Temperature: ");
  LoRa.print(temperature);
  LoRa.print(" °C, Humidity: ");
  LoRa.print(humidity);
  LoRa.print(" %");
  LoRa.endPacket();

  delay(10000); // Delay 10 detik
}