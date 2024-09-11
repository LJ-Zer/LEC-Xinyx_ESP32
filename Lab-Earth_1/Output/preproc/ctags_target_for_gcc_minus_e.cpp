# 1 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Lab-Earth_1.ino"
# 2 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Lab-Earth_1.ino" 2
# 3 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Lab-Earth_1.ino" 2
# 4 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Lab-Earth_1.ino" 2
# 5 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Lab-Earth_1.ino" 2
# 6 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Lab-Earth_1.ino" 2

// Pin definitions for LoRa and BME680




// Pin definitions for MiCS-6814 and MQ sensors
# 21 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Lab-Earth_1.ino"
// Pin definition for Analog rain sensor


Adafruit_BME680 bme;

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  while (!Serial);

  // Initialize BME680 sensor
  if (!bme.begin()) {
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("Could not find a valid BME680 sensor, check wiring!")))));
    while (1);
  }

  bme.setTemperatureOversampling(4 /*|< Alias for BME680 existing examples*/);
  bme.setHumidityOversampling(2 /*|< Alias for BME680 existing examples*/);
  bme.setPressureOversampling(3 /*|< Alias for BME680 existing examples*/);
  bme.setIIRFilterSize(2 /*|< Alias for BME680 existing examples*/);
  bme.setGasHeater(320, 150); // 320°C for 150 ms

  // Initialize LoRa transceiver module
  LoRa.setPins(5, 14, 2);
  if (!LoRa.begin(433E6)) { // Set the correct frequency for your module
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initialized");

  // Initialize rain sensor pin
  pinMode(13, 0x01);
}

void loop() {
  // Read analog value from rain sensor
  int rainValue = analogRead(13);

  // Print the rain sensor value to the serial monitor
  Serial.print("Rain Sensor Value: ");
  Serial.println(rainValue);

  // Start BME680 sensor reading
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("Failed to begin reading :(")))));
    return;
  }

  delay(50); // Give time for measurement

  if (!bme.endReading()) {
    Serial.println(((reinterpret_cast<const __FlashStringHelper *>(("Failed to complete reading :(")))));
    return;
  }

  // Retrieve BME680 sensor data
  float temp = bme.temperature;
  float pres = bme.pressure / 100.0; // Convert to hPa
  float hum = bme.humidity;
  float gas = bme.gas_resistance / 1000.0; // Convert to kOhms

  // Read analog values from MiCS-6814 sensors
  int co = analogRead(32 /* CO (MiCS-6814)*/);
  int no2 = analogRead(34 /* NO2 (MiCS-6814)*/);
  int nh3 = analogRead(35 /* NH3 (MiCS-6814)*/);

  // Read analog values from MQ sensors
  int mq4 = analogRead(33 /* MQ-4*/);
  int mq6 = analogRead(25 /* MQ-6*/);
  int mq8 = analogRead(26 /* MQ-8*/);
  int mq136 = analogRead(27 /* MQ-136*/);

  // Display BME680 sensor data on Serial Monitor
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

  // Display MiCS-6814 and MQ sensor data on Serial Monitor
  Serial.print("CO Value: ");
  Serial.print(co);
  Serial.print("\tNO2 Value: ");
  Serial.print(no2);
  Serial.print("\tNH3 Value: ");
  Serial.println(nh3);

  Serial.print("MQ-4 Value: ");
  Serial.print(mq4);
  Serial.print("\tMQ-6 Value: ");
  Serial.print(mq6);
  Serial.print("\tMQ-8 Value: ");
  Serial.print(mq8);
  Serial.print("\tMQ-136 Value: ");
  Serial.println(mq136);

  // Prepare data packet
  String dataPacket = "T:" + String(temp) + "C," +
                      "P:" + String(pres) + "hPa," +
                      "H:" + String(hum) + "%," +
                      "G:" + String(gas) + "KOhms," +
                      "CO:" + String(co) + "," +
                      "NO2:" + String(no2) + "," +
                      "NH3:" + String(nh3) + "," +
                      "MQ4:" + String(mq4) + "," +
                      "MQ6:" + String(mq6) + "," +
                      "MQ8:" + String(mq8) + "," +
                      "MQ136:" + String(mq136) + "," +
                      "Rain:" + String(rainValue);

  // Send packet over LoRa
  Serial.print("Sending packet: ");
  Serial.println(dataPacket);

  LoRa.beginPacket();
  LoRa.print(dataPacket);
  LoRa.endPacket();

  delay(5000); // Wait before sending next packet
}
