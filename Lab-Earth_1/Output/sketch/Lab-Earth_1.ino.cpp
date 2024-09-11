#include <Arduino.h>
#line 1 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Lab-Earth_1.ino"
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

// Pin definitions for LoRa and BME680
#define SS 5
#define RST 14
#define DI0 2

// Pin definitions for MiCS-6814 and MQ sensors
#define CO_PIN 32     // CO (MiCS-6814)
#define NO2_PIN 34    // NO2 (MiCS-6814)
#define NH3_PIN 35    // NH3 (MiCS-6814)
#define MQ4_PIN 33    // MQ-4
#define MQ6_PIN 25    // MQ-6
#define MQ8_PIN 26    // MQ-8
#define MQ136_PIN 27  // MQ-136

// Pin definition for Analog rain sensor
#define RAIN_SENSOR_PIN 13

Adafruit_BME680 bme;

#line 26 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Lab-Earth_1.ino"
void setup();
#line 55 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Lab-Earth_1.ino"
void loop();
#line 26 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Lab-Earth_1.ino"
void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  while (!Serial);

  // Initialize BME680 sensor
  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320°C for 150 ms

  // Initialize LoRa transceiver module
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(433E6)) {  // Set the correct frequency for your module
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initialized");

  // Initialize rain sensor pin
  pinMode(RAIN_SENSOR_PIN, INPUT);
}

void loop() {
  // Read analog value from rain sensor
  int rainValue = analogRead(RAIN_SENSOR_PIN);

  // Print the rain sensor value to the serial monitor
  Serial.print("Rain Sensor Value: ");
  Serial.println(rainValue);

  // Start BME680 sensor reading
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }

  delay(50); // Give time for measurement

  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }

  // Retrieve BME680 sensor data
  float temp = bme.temperature;
  float pres = bme.pressure / 100.0; // Convert to hPa
  float hum = bme.humidity;
  float gas = bme.gas_resistance / 1000.0; // Convert to kOhms

  // Read analog values from MiCS-6814 sensors
  int co = analogRead(CO_PIN);
  int no2 = analogRead(NO2_PIN);
  int nh3 = analogRead(NH3_PIN);

  // Read analog values from MQ sensors
  int mq4 = analogRead(MQ4_PIN);
  int mq6 = analogRead(MQ6_PIN);
  int mq8 = analogRead(MQ8_PIN);
  int mq136 = analogRead(MQ136_PIN);

  // Display BME680 sensor data on Serial Monitor
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

