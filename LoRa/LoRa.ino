#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

// Pin definitions
#define SS 5
#define RST 14
#define DI0 2

Adafruit_BME680 bme; 

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

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

  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }

  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);

  Serial.println(F("You can do other work during BME680 measurement."));
  delay(50); 

  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  Serial.print(F("Reading completed at "));
  Serial.println(millis());

  // Retrieve sensor data
  float temp = bme.temperature;
  float pres = bme.pressure / 100.0; // Convert to hPa
  float hum = bme.humidity;
  float gas = bme.gas_resistance / 1000.0; // Convert to kOhms

  // Display sensor data on Serial Monitor
  Serial.print(F("Temperature = "));
  Serial.print(temp);
  Serial.println(F(" °C"));

  Serial.print(F("Pressure = "));
  Serial.print(pres);
  Serial.println(F(" hPa"));

  Serial.print(F("Humidity = "));
  Serial.print(hum);
  Serial.println(F(" %"));

  Serial.print(F("Gas = "));
  Serial.print(gas);
  Serial.println(F(" KOhms"));

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

