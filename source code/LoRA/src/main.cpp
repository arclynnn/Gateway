#include <SPI.h>
#include <LoRa.h>
#include <LowPower.h>
// "ArduinoLowPower.h"

const int csPin = 10;
const int resetPin = 7;
const int irqPin = 2;
String dataconfirm = "";
String sensorData;
String dataheader = "s1";
String waktutidur = "";
char datachar;

byte localAddress = 0xAA;
byte destinationAddress = 0xBB;
long lastSendTime = 0;
int interval = 100;
long lastSendTimeSerial = 0;
int intervalSerial = 100;
int count = 0;
bool stringComplete = false;
String inputString = ""; 
int pinOut = 11;
String datakonfirmttl = "1";

int counterlora =0;
int counterttl = 0;


void programsleep() {
  for (int i = 0; i < 72; i++)  // i*8s sleep time
  {
    // LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF,
    //               SPI_OFF, USART0_OFF, TWI_OFF);

    LowPower.idle(SLEEP_8S, ADC_OFF, TIMER5_OFF, TIMER4_OFF, TIMER3_OFF, 
  		  TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART3_OFF, 
  		  USART2_OFF, USART1_OFF, USART0_OFF, TWI_OFF);
  }
  waktutidur = "";
  counterlora = 0;
  counterttl = 0;
  dataconfirm = "";
  sensorData = "";
  stringComplete = false;
  Serial1.flush();
  Serial.print("sensorData = ");
  Serial.println(sensorData);
  
  Serial.println("data kereset semua");
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();
  LoRa.write(destinationAddress);  //0xBB
  LoRa.write(localAddress);        //0xAA
  LoRa.write(outgoing.length());   //panjang data
  LoRa.print(outgoing);            //print data
  LoRa.endPacket();
}

void kirimkonfirm()
{
    Serial1.println(datakonfirmttl);
}

void receiveMessage(int packetSize) {
  if (packetSize == 0) return;

   int recipient = LoRa.read();
  // byte sender = LoRa.read();
  // byte incomingLength = LoRa.read();

  String incoming = "";

  while (LoRa.available()) 
  {
    incoming += (char)LoRa.read();
  //    Serial.print((char)LoRa.read());
  }

  // if (incomingLength != incoming.length()) {
  //   Serial.println("Error: Message length does not match length");
  //   return;
  // }

   if (recipient != localAddress) {
     Serial.println("Error: Recipient address does not match local address");
     return;
   }

  Serial.print("Received data " + incoming);
//  Serial.print(" from 0x" + String(sender, HEX));
   Serial.println(" to 0x" + String(recipient, HEX));
  dataconfirm = incoming;
  delay(1000);
}

void relay()
{
  if (waktutidur == ""){
    digitalWrite(pinOut, HIGH);  
  }
  else {
    digitalWrite(pinOut, LOW);
  }
}

void setup() {
  Serial1.begin(9600);
  Serial.begin(9600);
  Serial.println("Start LoRa duplex");

  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(920E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (true) {}
  }

  pinMode(11, OUTPUT); //pin control relay 
}

void loop() {
  delay(100);
  
  if (waktutidur == "")
  {
    digitalWrite(pinOut, HIGH);  
  }
  else 
  {
    digitalWrite(pinOut, LOW);
  }
  
  Serial1.begin(9600);
  
  delay(200);   // memberikan waktu untuk master node mengambil data
  
  Serial.println("Menunggu data sensor");
  
  while (Serial1.available()) {  // if there's incoming serial data
    char inChar = (char)Serial1.read();  // read it
    if (inChar == 's') {        // if the incoming character is a newline
      stringComplete = false;     // set stringComplete to true
      sensorData += inChar;
    } 
    else if (inChar == '\n'){
      stringComplete = true;
      Serial.print("String : ");
      Serial.println(sensorData);
      break;
    }
    else 
    {
      if(!stringComplete) sensorData += inChar;
    }
  }
   
  if (counterttl >75)
  {
    kirimkonfirm();
    digitalWrite(pinOut, LOW);
    waktutidur = "1";
    Serial.println("data ttl melebihi batas, sleep");
    delay(100);
   
    programsleep();
    return;
    
  }
  else if(dataconfirm =! "ok" && stringComplete == true)
  {
      String firstTwo = sensorData.substring(0,2);
      Serial.println(sensorData);
      
      if (sensorData == "")
      {
        Serial.println("data belum diterima");
        delay(500);
        counterttl++;
        return;
      }
      else if (dataheader != firstTwo)
      {
        Serial.println("data header salah");
        delay(500);
        counterttl++;
        return;
      }
      else if (sensorData.length() < 3)  //dihitung dari panjang data
      {
        delay(500);
        counterttl++;
        return;
      }
      else
      {
        kirimkonfirm();
      }
      
      while (dataconfirm != "ok") 
      {
        if (millis() - lastSendTime > interval) 
        {
          // String sensorData = "123,1234567890,12.1,30,96,1234,1234,96,100,100,123,96";
          Serial.print(sensorData);
          sendMessage(sensorData);

          Serial.print("Sending data " + sensorData);
          Serial.print(" from 0x" + String(localAddress, HEX));
          Serial.println(" to 0x" + String(destinationAddress, HEX));

          lastSendTime = millis();
          interval = random(2000) + 1000;
          counterlora++;
        }
        else if(counterlora>30)
        {
          digitalWrite(pinOut, LOW);
          waktutidur = "1";
          Serial.println("datalora melebihi batas, sleep");
          delay(100);
          programsleep();
          return;
        }
      receiveMessage(LoRa.parsePacket());
      
      }
   }
   else
   {
     delay(100);
     return;
   }

  Serial.println("Terima dataconfirm : " + dataconfirm);
  Serial.println("Sleep mas Bro dan Perintah relay open");
  digitalWrite(pinOut, LOW);
  waktutidur = "1";

  delay(1000);
  programsleep();

  Serial.println("Bangun Sleep");
  delay(1000);

}
