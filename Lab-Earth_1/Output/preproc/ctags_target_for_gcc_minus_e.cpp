# 1 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\LoRa\\LoRa.ino"
# 2 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\LoRa\\LoRa.ino" 2
# 3 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\LoRa\\LoRa.ino" 2
# 4 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\LoRa\\LoRa.ino" 2
# 5 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\LoRa\\LoRa.ino" 2
# 6 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\LoRa\\LoRa.ino" 2

// Pin definitions




Adafruit_BME680 bme;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!bme.begin()) {
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("Could not find a valid BME680 sensor, check wiring!")))));
    while (1);
  }

  bme.setTemperatureOversampling(4 /*|< Alias for BME680 existing examples*/);
  bme.setHumidityOversampling(2 /*|< Alias for BME680 existing examples*/);
  bme.setPressureOversampling(3 /*|< Alias for BME680 existing examples*/);
  bme.setIIRFilterSize(2 /*|< Alias for BME680 existing examples*/);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  Serial.println("LoRa Sender");

  // setup LoRa transceiver module
  LoRa.setPins(5, 14, 2);
  if (!LoRa.begin(433E6)) { // Set the correct frequency for your module
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initialized");
}

void loop() {

  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("Failed to begin reading :(")))));
    return;
  }

  Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("Reading started at ")))));
  Serial.print(millis());
  Serial.print(((reinterpret_cast<const __FlashStringHelper *>((" and will finish at ")))));
  Serial.println(endTime);

  Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("You can do other work during BME680 measurement.")))));
  delay(50);

  if (!bme.endReading()) {
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("Failed to complete reading :(")))));
    return;
  }
  Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("Reading completed at ")))));
  Serial.println(millis());

  // Retrieve sensor data
  float temp = bme.temperature;
  float pres = bme.pressure / 100.0; // Convert to hPa
  float hum = bme.humidity;
  float gas = bme.gas_resistance / 1000.0; // Convert to kOhms

  // Display sensor data on Serial Monitor
  Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("Temperature = ")))));
  Serial.print(temp);
  Serial.println(((reinterpret_cast<const __FlashStringHelper *>((" °C")))));

  Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("Pressure = ")))));
  Serial.print(pres);
  Serial.println(((reinterpret_cast<const __FlashStringHelper *>((" hPa")))));

  Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("Humidity = ")))));
  Serial.print(hum);
  Serial.println(((reinterpret_cast<const __FlashStringHelper *>((" %")))));

  Serial.print(((reinterpret_cast<const __FlashStringHelper *>(("Gas = ")))));
  Serial.print(gas);
  Serial.println(((reinterpret_cast<const __FlashStringHelper *>((" KOhms")))));

  // Prepare data packet
  String dataPacket = "T:" + String(temp) + "C," +
                      "P:" + String(pres) + "hPa," +
                      "H:" + String(hum) + "%," +
                      "G:" + String(gas) + "KOhms";

  // Send packet over LoRa
  Serial.print("Sending packet: ");
  Serial.println(dataPacket);

  LoRa.beginPacket();
  LoRa.print(dataPacket);
  LoRa.endPacket();

  delay(5000); // Wait before sending next packet
}
