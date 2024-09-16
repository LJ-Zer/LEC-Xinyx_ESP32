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
#define cmn 32    // CO (MiCS-6814)
#define ngd 34    // NO2 (MiCS-6814)
#define amn 35    // NH3 (MiCS-6814)
#define mtn 33    // MQ-4 (Methane - CH4)
#define lpg 25    // MQ-6 (LPG)
#define hgn 26    // MQ-8 (Hydrogen - H2)
#define slfr 27   // MQ-136 (Sulfur Dioxide - SO2)

// Pin definition for Analog rain sensor
#define rain 13

const int voltageSensor = 12; // solar sensor
float vOUT = 0.0;
float vIN = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
int value = 0;

int RelayPin = 17; // solar relay

Adafruit_BME680 bme;

// Calibration constants for MQ sensors
float R0_SO2 = 2000.0;  // Clean air resistance for SO2 (MQ-136)
float S_SO2 = 3.0;      // Sensitivity factor for SO2

float R0_H2 = 10000.0;  // Clean air resistance for H2 (MQ-8)
float S_H2 = 0.6;       // Sensitivity factor for H2

float R0_LPG = 10000.0; // Clean air resistance for LPG (MQ-6)
float S_LPG = 0.6;      // Sensitivity factor for LPG

float R0_CH4 = 10000.0; // Clean air resistance for Methane (MQ-4)
float S_CH4 = 0.6;      // Sensitivity factor for Methane

// Calibration constants for MiCS-6814
float R0_CO = 150.0;    // Clean air resistance for CO (MiCS-6814)
float R0_NO2 = 1.0;     // Clean air resistance for NO2 (MiCS-6814)
float R0_NH3 = 1500.0;  // Clean air resistance for NH3 (MiCS-6814)

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  while (!Serial);

  pinMode(RelayPin, OUTPUT);
  digitalWrite(RelayPin, LOW);

  pinMode(rain, INPUT);

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
}

void loop() {
  // Read analog value from rain sensor
  int rainValue = analogRead(rain);

  // Print the rain sensor value to the serial monitor
  Serial.print("Rain Sensor Value: ");
  Serial.println(rainValue);

  // Solar sensor reading and calculation
  value = analogRead(voltageSensor);
  vOUT = (value * 3.3) / 1024.0;
  vIN = ((vOUT / (R2 / (R1 + R2))) / 3.3) - 3.6;
  Serial.print("Input Solar Voltage: ");
  Serial.println(vIN);
  delay(1000);

  if (vIN <= 3.5) {
    digitalWrite(RelayPin, LOW);
    delay(100);
  }
  if (vIN >= 4) {
    digitalWrite(RelayPin, HIGH);
    delay(100);
  }

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

  // Read analog values from MQ and MiCS-6814 sensors
  int so2_raw = analogRead(slfr);   // SO2
  int h2_raw = analogRead(hgn);     // H2
  int lpg_raw = analogRead(lpg);    // LPG
  int ch4_raw = analogRead(mtn);    // Methane (CH4)
  int co_raw = analogRead(cmn);     // CO (MiCS-6814)
  int no2_raw = analogRead(ngd);    // NO2 (MiCS-6814)
  int nh3_raw = analogRead(amn);    // NH3 (MiCS-6814)

  // Calculate sensor resistance (Rs) based on raw values
  float Rs_SO2 = so2_raw;
  float Rs_H2 = h2_raw;
  float Rs_LPG = lpg_raw;
  float Rs_CH4 = ch4_raw;
  float Rs_CO = co_raw;
  float Rs_NO2 = no2_raw;
  float Rs_NH3 = nh3_raw;

  // Convert to ppm using calibration formulas
  float ppm_SO2 = (Rs_SO2 / R0_SO2) * S_SO2 * 50.0;   // 50 ppm SO2 is the reference
  float ppm_H2 = (Rs_H2 / R0_H2) * S_H2 * 1000.0;     // 1000 ppm H2 is the reference
  float ppm_LPG = (Rs_LPG / R0_LPG) * S_LPG * 1000.0; // 1000 ppm CH4 is the reference
  float ppm_CH4 = (Rs_CH4 / R0_CH4) * S_CH4 * 1000.0; // 1000 ppm CH4 is the reference

  // MiCS-6814 calculations (calibrate for CO, NO2, NH3)
  float ppm_CO = (Rs_CO / R0_CO) * 1000.0;   // CO range is 1–1000 ppm
  float ppm_NO2 = (Rs_NO2 / R0_NO2) * 10.0;  // NO2 range is 0.05–10 ppm
  float ppm_NH3 = (Rs_NH3 / R0_NH3) * 500.0; // NH3 range is 1–500 ppm

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

  // Display calibrated MiCS-6814 sensor data
  Serial.print("CO Concentration: ");
  Serial.print(ppm_CO);
  Serial.println(" ppm");

  Serial.print("NO2 Concentration: ");
  Serial.print(ppm_NO2);
  Serial.println(" ppm");

  Serial.print("NH3 Concentration: ");
  Serial.print(ppm_NH3);
  Serial.println(" ppm");

  // Display calibrated MQ sensor data
  Serial.print("SO2 Concentration: ");
  Serial.print(ppm_SO2);
  Serial.println(" ppm");

  Serial.print("H2 Concentration: ");
  Serial.print(ppm_H2);
  Serial.println(" ppm");

  Serial.print("LPG Concentration: ");
  Serial.print(ppm_LPG);
  Serial.println(" ppm");

  Serial.print("Methane (CH4) Concentration: ");
  Serial.print(ppm_CH4);
  Serial.println(" ppm");

  // Prepare data packet
  String dataPacket = "T:" + String(temp) + "C," +
                      "P:" + String(pres) + "hPa," +
                      "H:" + String(hum) + "%," +
                      "G:" + String(gas) + "KOhms," +
                      "CO:" + String(ppm_CO) + "ppm," +
                      "NO2:" + String(ppm_NO2) + "ppm," +
                      "NH3:" + String(ppm_NH3) + "ppm," +
                      "SO2:" + String(ppm_SO2) + "ppm," +
                      "H2:" + String(ppm_H2) + "ppm," +
                      "LPG:" + String(ppm_LPG) + "ppm," +
                      "CH4:" + String(ppm_CH4) + "ppm";

  // Send data over LoRa
  LoRa.beginPacket();
  LoRa.print(dataPacket);
  LoRa.endPacket();
  
  // Wait before next reading
  delay(5000);
}
