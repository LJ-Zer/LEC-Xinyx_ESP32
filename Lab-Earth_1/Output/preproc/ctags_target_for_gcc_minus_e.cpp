# 1 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\RLoRa\\RLoRa.ino"
# 2 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\RLoRa\\RLoRa.ino" 2
# 3 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\RLoRa\\RLoRa.ino" 2





void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");

  LoRa.setPins(5, 14, 2); //setup LoRa transceiver module

  while (!LoRa.begin(433E6)) //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xA5);
  Serial.println("LoRa Initializing OK!");
}

void loop()
{
  int packetSize = LoRa.parsePacket(); // try to parse packet
  if (packetSize)
  {

    Serial.print("Received packet '");

    while (LoRa.available()) // read packet
    {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData);
    }
    Serial.print("' with RSSI "); // print RSSI of packet
    Serial.println(LoRa.packetRssi());
  }
}
