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
#define cmn 34     // CO (MiCS-6814)
#define ngd 35    // NO2 (MiCS-6814)
#define amn 32    // NH3 (MiCS-6814)
#define mtn 33    // MQ-4
#define lpg 25    // MQ-6
#define hgn 26    // MQ-8
#define slfr 27  // MQ-136

// Pin definition for Analog rain sensor
#define rain 13

/////////////////////////////////////////////////////////
// Constants
const int hallSensorPin = 0;  // Pin connected to the Hall sensor (digital pin 2)
const float radius = 0.15;    // Radius of the anemometer's rotating part in meters (adjust as per your device)
const float calibrationFactor = 1.2;  // Calibration factor depending on anemometer (adjust as needed)

// Variables
volatile int rotationCount = 0;
unsigned long lastMillis = 0;
unsigned long interval = 2000; // Measurement interval (e.g., 2 seconds)
float windSpeed = 0;
////////////////////////////////////////////////////////

const int voltageSensor = 12; //solar sensor
const int wdrctn = 4; //wind direction
float vOUT = 0.0;
float vIN = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
int value = 0;
int value2 = 0;
float drctn = 0.0;
float vOUT1 = 0.0;
int RelayPin = 17; // solar relay

Adafruit_BME680 bme;

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  while (!Serial);

  pinMode(RelayPin, OUTPUT);
  digitalWrite(RelayPin, LOW);

  pinMode(rain, INPUT);

  pinMode(hallSensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin), countRotation, FALLING);

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

  unsigned long currentMillis = millis();
  
  // Check if it's time to calculate wind speed
  if (currentMillis - lastMillis >= interval) {
    // Calculate wind speed
    float rotationsPerSecond = rotationCount / (interval / 1000.0); // rotations per second
    float windSpeed = calculateWindSpeed(rotationsPerSecond); // in m/s

    // Output results
    Serial.print("Wind Speed: ");
    Serial.print(windSpeed);
    Serial.println(" m/s");

    // Reset count and time
    rotationCount = 0;
    lastMillis = currentMillis;
  }

  value = analogRead(voltageSensor);
  vOUT = (value * 3.3) / 1024.0;
  vIN = ((vOUT / (R2/(R1+R2)))/3.3);
  Serial.print("Input Solar Voltage: ");
  Serial.println(vIN);
  delay(100);

  if (vIN <= 3.5) {
    digitalWrite(RelayPin, LOW);
    delay(100);
  }
  if (vIN >= 4) {
    digitalWrite(RelayPin, HIGH);
    delay(100);    
  }
  //////////////////////////////////////////

  value2 = analogRead(wdrctn);
  vOUT1 = (value2 * 3.3) / 1024.0;
  drctn = ((vOUT1 / (R2/(R1+R2)))/3.3);
  Serial.print("Drctn_Vltg: ");
  Serial.println(drctn);
  delay(100);
  
  ////////////////////////////////////////////

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
  int co = analogRead(cmn);
  int no2 = analogRead(ngd);
  int nh3 = analogRead(amn);

  // Read analog values from MQ sensors
  int mq4 = analogRead(mtn);
  int mq6 = analogRead(lpg);
  int mq8 = analogRead(hgn);
  int mq136 = analogRead(slfr);

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
                      "Rain:" + String(rainValue) + "," +
                      "Svlts:" + String(vIN) + "," +
                      "DrctnVolts:" + String(drctn) + "," +
                      "WS:" + String(windSpeed);

  // Send packet over LoRa
  Serial.print("Sending packet: ");
  Serial.println(dataPacket);

  LoRa.beginPacket();
  LoRa.print(dataPacket);
  LoRa.endPacket();

  Serial.print("Data packet length: ");
  Serial.println(dataPacket.length());


  delay(3000); // Wait before sending next packet
}

// Interrupt function to count each rotation
void countRotation() {
  rotationCount++;
}

// Function to calculate wind speed from rotations per second
float calculateWindSpeed(float rotationsPerSecond) {
  float circumference = 2 * PI * radius; // Circumference of the rotating part
  float windSpeed = (rotationsPerSecond * circumference) * calibrationFactor; // Speed = rotations/sec * circumference
  return windSpeed;
}
