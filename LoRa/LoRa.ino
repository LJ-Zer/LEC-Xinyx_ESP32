#include <SPI.h>
#include <LoRa.h>

// Pin definitions
#define SS 5
#define RST 14
#define DI0 2

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Sender");

  // setup LoRa transceiver module
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(433E6)) {  // Set the correct frequency for your module
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initialized");
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println("Hello, LoRa!");

  // send packet
  LoRa.beginPacket();
  LoRa.print("Hello, LoRa!");
  LoRa.endPacket();

  delay(1000);  // wait for a second before sending the next packet
}
