#include <Arduino.h>
 
#include <SPI.h>
#include <Ethernet.h>
#include <LoRa.h>
#include <PubSubClient.h>

#define LORA_SCK    5
#define LORA_MISO   19
#define LORA_MOSI   27
#define LORA_CS     18
#define LORA_RST    23
#define LORA_DIO0   26

#define LORA_FREQ   868000000
#define LORA_BAND   125E5
#define LORA_SF     8
#define LORA_RATE   4
#define LORA_PREA   8
#define LORA_SYNC   0x34

#define ETH_SCK     14
#define ETH_MISO    25
#define ETH_MOSI    13
#define ETH_CS      15
 
#define IPADDR      192,168,23,100
#define IPMASK      255,255,255,0
#define DNS         192,168,23,1
#define GATEWAY     192,168,23,1

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
unsigned long previousMillis = 0;
String clientId = "T-BEAM-W5500";

IPAddress ip(IPADDR);
IPAddress sn(IPMASK);
IPAddress dns(DNS);
IPAddress gw(GATEWAY);
IPAddress server(GATEWAY);

void callback(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);
SPIClass LoRa_SPI = SPIClass(HSPI);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // do some length checks here
  byte* p = (byte*)malloc(length);
  memcpy(p, payload, length);

  // echo back the packet on outTopic
  client.publish("outTopic", p, length);

  // send received MQTT packet via LoRa
  LoRa.beginPacket();
  LoRa.write(p, length);
  LoRa.endPacket();

  // do not forget to free allocated memory
  free(p);
}

void initLoRa(){
  LoRa_SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);

  // Important: Pass the LoRa_SPI to the setSPI function!
  LoRa.setSPI(LoRa_SPI);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);
  LoRa.setFrequency(LORA_FREQ);
  LoRa.setSignalBandwidth(LORA_BAND);

  if (!LoRa.begin(LORA_FREQ)) {
    while (true) {
      Serial.println("Starting LoRa failed!");
      delay(5000);
    }
  } else {
    Serial.println("LoRa OK");
  }

  // Settings for LoRaWAN compatible LoRa packets
  LoRa.enableCrc();
  LoRa.setCodingRate4(LORA_RATE);
  LoRa.setPreambleLength(LORA_PREA);
  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setSyncWord(LORA_SYNC);
  LoRa.enableInvertIQ(); // Prevent other LoRaWAN Gateways from receiving downstream messages
}

void initEthernet() {
  Serial.println("Begin Ethernet");
  
  SPI.begin(ETH_SCK, ETH_MISO, ETH_MOSI, ETH_CS);
  Ethernet.init(ETH_CS);
  Ethernet.begin(mac, ip, dns, gw, sn);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    while (true) {
      Serial.println("Ethernet shield was not found.");
      delay(5000);
    }
  }

  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  Serial.println("Ethernet Successfully Initialized");

  if (client.connect(clientId.c_str())) {
    client.publish("outTopic","Hello Technopolis Citizen");
    client.subscribe("inTopic");
  }
}

void reconnect() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("outTopic","Hello Technopolis Citizen");
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
  }
}

void setup() {
  Serial.begin(115200);

  initEthernet();
  initLoRa(); 
}
 
void loop() {
  unsigned long currentMillis = millis();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (currentMillis - previousMillis >= 15000) {
    Serial.print("Sending Test LoRa packet... ");
    LoRa.beginPacket();
    LoRa.print("OK");
    LoRa.endPacket();
    Serial.println("OK");

    // uncomment if you want to send MQTT test packet too
    // if (Ethernet.linkStatus() == LinkON) {
    //   if (client.connect(clientId.c_str())) {
    //     client.publish("outTopic", "OK");
    //   }
    // }
   
    previousMillis = currentMillis;
  }
}
