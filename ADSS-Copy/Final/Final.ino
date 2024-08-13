#include <connection.h>
#include <WiFi.h>
#include <HTTPUpdate.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <cert.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <SPI.h>

int Northdir = 4;
int NorthEastdir = 5;
int Eastdir = 27;
int SouthEastdir = 14;
int Southdir = 26;
int SouthWestdir = 19;
int Westdir = 34;
int NorthWestdir = 35;

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;

const int speedsense = 16;
const float Diameter = 0.065;
const int hpr = 20;
volatile unsigned int holes = 0;
unsigned long previousMillis = 0;
unsigned long interval = 1000;
float rpm = 0.0;
float windspeed = 0.0;

#define precipita 13

const int voltageSensor = 32;
float vOUT = 0.0;
float vIN = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
int value = 0;

int RelayPin = 15;

void setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WifiName, WifiPassword);
  Serial.begin(115200);
  pinMode(Northdir, INPUT);
  pinMode(NorthEastdir, INPUT);
  pinMode(Eastdir, INPUT);
  pinMode(SouthEastdir, INPUT);
  pinMode(Southdir, INPUT);
  pinMode(SouthWestdir, INPUT);
  pinMode(Westdir, INPUT);
  pinMode(NorthWestdir, INPUT);

  !bme.begin(0x76);

  pinMode(precipita, INPUT);

  pinMode(speedsense, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(speedsense), HolesCount, RISING);

  pinMode(RelayPin, OUTPUT);
  digitalWrite(RelayPin, LOW);
}

void loop()
{
  WiFi.disconnect(WifiName, WifiPassword);
  int North = digitalRead(Northdir);
  int NorthEast = digitalRead(NorthEastdir);
  int East = digitalRead(Eastdir);
  int SouthEast = digitalRead(SouthEastdir);
  int South = digitalRead(Southdir);
  int SouthWest = digitalRead(SouthWestdir);
  int West = digitalRead(Westdir);
  int NorthWest = digitalRead(NorthWestdir);

  Serial.print ("Num4: ");
  Serial.println(North);
  Serial.print ("Num5: ");
  Serial.println(NorthEast);
  Serial.print ("Num27: ");
  Serial.println(East);
  Serial.print ("Num14: ");
  Serial.println(SouthEast);
  Serial.print ("Num26: ");
  Serial.println(South);
  Serial.print ("Num19: ");
  Serial.println(SouthWest);
  Serial.print ("Num34: ");
  Serial.println(West);
  Serial.print ("Num35: ");
  Serial.println(NorthWest);

  int dista;

  if (North == 0 && NorthEast == 1 && East == 1 && SouthEast == 1 && South == 1 && SouthWest == 1 && West == 1 && NorthWest == 1)
  {
    Serial.println("Direction: North");
    dista = 1;
  }
  else if (North == 1 && NorthEast == 0 && East == 1 && SouthEast == 1 && South == 1 && SouthWest == 1 && West == 1 && NorthWest == 1)
  {
    Serial.println("Direction: NorthEast");
    dista = 2;
  }
  else if (North == 1 && NorthEast == 1 && East == 0 && SouthEast == 1 && South == 1 && SouthWest == 1 && West == 1 && NorthWest == 1)
  {
    Serial.println("Direction: East");
    dista = 3;
  }
  else if (North == 1 && NorthEast == 1 && East == 1 && SouthEast == 0 && South == 1 && SouthWest == 1 && West == 1 && NorthWest == 1)
  {
    Serial.println("Direction: SouthEast");
    dista = 4;
  }
  else if (North == 1 && NorthEast == 1 && East == 1 && SouthEast == 1 && South == 0 && SouthWest == 1 && West == 1 && NorthWest == 1)
  {
    Serial.println("Direction: South");
    dista = 5;
  }
  else if (North == 1 && NorthEast == 1 && East == 1 && SouthEast == 1 && South == 1 && SouthWest == 0 && West == 1 && NorthWest == 1)
  {
    Serial.println("Direction: SouthWest");
    dista = 6;
  }
  else if (North == 1 && NorthEast == 1 && East == 1 && SouthEast == 1 && South == 1 && SouthWest == 1 && West == 0 && NorthWest == 1)
  {
    Serial.println("Direction: West");
    dista = 7;
  }
  else if (North == 1 && NorthEast == 1 && East == 1 && SouthEast == 1 && South == 1 && SouthWest == 1 && West == 1 && NorthWest == 0)
  {
    Serial.println("Direction: NorthWest");
    dista = 8;
  }
  else
  {
    Serial.println("");
    dista = 0;
  }
  Serial.print("Distance Code: ");
  Serial.print(dista);
  delay(1000);

  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" *C");
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" hPa");
  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println(" %");

  delay(1000);

  float Prep = analogRead(precipita);
  float precipitations = ((4095 - Prep) * 100 / 4095);
  Serial.print("Precipitation: ");
  Serial.print(precipitations);
  //Serial.println(precipita);
  delay(1000);

  value = analogRead(voltageSensor);
  vOUT = (value * 3.3) / 1024.0;
  vIN = (vOUT / (R2 / (R1 + R2))) / 3.3;
  Serial.print("Input Voltage: ");
  Serial.println(vIN);
  delay(1000);
  if (vIN >= 4)
  {
    digitalWrite(RelayPin, LOW);
    delay(100);
  }
  if (vIN <= 5)
  {
    digitalWrite(RelayPin, HIGH);
    delay(100);
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    WiFi.disconnect(WifiName, WifiPassword);
    previousMillis = currentMillis;
    detachInterrupt(speedsense);
    rpm = holes * (60.0 / hpr);
    windspeed = rpm * Diameter * 3.14159 / 60.0;
    holes = 0;
    delay(10);
    attachInterrupt(digitalPinToInterrupt(speedsense), HolesCount, RISING);
    Serial.print("Speed: ");
    Serial.print(windspeed, 2);
    Serial.println(" m/s");
    delay(1000);

    WiFi.begin(WifiName, WifiPassword);
    delay(7000);

    write_to_google_sheet(temperature, humidity, pressure, windspeed, precipitations, dista);
  }


}

void write_to_google_sheet(float temperature, float humidity, float pressure, float windspeed, float precipitations, float dista)
{
  String SCRIPT_ID = "AKfycbzpEMJ-52ayiOc1Uw_JjZ8SHw6ganyoc_l8HYI2TYqxz4eN4zhQiapOOldazMlWfrXt";
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + SCRIPT_ID + "/exec?temperature=" + String(temperature) + "&pressure=" + String(pressure) + "&humidity=" + String(humidity) + "&windspeed=" + String(windspeed) + "&precipitations=" + String(precipitations) + "&dista=" + String(dista) + "&vIN=" + String(vIN);
  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();

  String payload;
  http.end();
}

void HolesCount()
{
  holes += 1;
}
