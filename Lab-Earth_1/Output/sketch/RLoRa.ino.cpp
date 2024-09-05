#include <Arduino.h>
#line 1 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\RLoRa\\RLoRa.ino"
#include <SPI.h>
#include <LoRa.h>

// Pin definitions
#define SS 5
#define RST 14
#define DI0 2

#line 9 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\RLoRa\\RLoRa.ino"
void setup();
#line 24 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\RLoRa\\RLoRa.ino"
void loop();
#line 9 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\RLoRa\\RLoRa.ino"
void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Receiver");

  // setup LoRa transceiver module
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(433E6)) {  // Set the correct frequency for your module
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initialized");
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet: '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}

