#include <Arduino.h>
#line 1 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_2\\Lab-Earth_2.ino"
#include <SPI.h>
#include <LoRa.h>
#include <HTTPClient.h>
#include <WiFi.h>

// Pin definitions for LoRa module
#define SS 5
#define RST 14
#define DI0 2

char ssid[] = "Smart_Bro_0E109";
char pass[] = "DiosteBendiga08";

#line 14 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_2\\Lab-Earth_2.ino"
void setup();
#line 36 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_2\\Lab-Earth_2.ino"
void loop();
#line 101 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_2\\Lab-Earth_2.ino"
void Sheet(float temp, float pres, float hum, float gas, int co, int no2, int nh3, int so2, int h2, int lpg, int ch4, int rain);
#line 14 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_2\\Lab-Earth_2.ino"
void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  while (!Serial);

  Serial.println("LoRa Receiver");

  // Setup LoRa transceiver module
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(433E6)) {  // Set the correct frequency for your module
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initialized");
}

void loop() {
  // Try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Received a packet
    Serial.print("Received packet: ");

    // Read packet
    String receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    
    // Print the received data
    Serial.println(receivedData);

    // Optionally, parse the data if it's in a known format
    // For example, if the format is "T:XX.XXC,P:XX.XXhPa,H:XX.XX%,G:XX.XXKOhms,CO:XX,NO2:XX,NH3:XX,SO2:XX,H2:XX,LPG:XX,CH4:XX,Rain:XX"
    float temp, pres, hum, gas;
    int co, no2, nh3, so2, h2, lpg, ch4, rain;
    sscanf(receivedData.c_str(), 
           "T:%fC,P:%fhPa,H:%f%%,G:%fKOhms,CO:%d,NO2:%d,NH3:%d,SO2:%d,H2:%d,LPG:%d,CH4:%d,Rain:%d",
           &temp, &pres, &hum, &gas, &co, &no2, &nh3, &so2, &h2, &lpg, &ch4, &rain);

    // Display parsed data
    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" °C");

    Serial.print("Pressure: ");
    Serial.print(pres);
    Serial.println(" hPa");

    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.println(" %");

    Serial.print("Gas Resistance: ");
    Serial.print(gas);
    Serial.println(" KOhms");

    Serial.print("CO Concentration: ");
    Serial.print(co);
    Serial.print("\tNO2 Concentration: ");
    Serial.print(no2);
    Serial.print("\tNH3 Concentration: ");
    Serial.print(nh3);
    Serial.print("\tSO2 Concentration: ");
    Serial.print(so2);
    Serial.print("\tH2 Concentration: ");
    Serial.print(h2);
    Serial.print("\tLPG Concentration: ");
    Serial.print(lpg);
    Serial.print("\tMethane (CH4) Concentration: ");
    Serial.print(ch4);
    Serial.print("\tRain Sensor Value: ");
    Serial.println(rain);

    Serial.println(); // Add an empty line for readability

    // Send data to Google Sheets
    Sheet(temp, pres, hum, gas, co, no2, nh3, so2, h2, lpg, ch4, rain);
  }
}

void Sheet(float temp, float pres, float hum, float gas, int co, int no2, int nh3, int so2, int h2, int lpg, int ch4, int rain) 
{
   String SCRIPT_ID = "AKfycbyD_rDwg4TJrIRw4yNzEXGvgJshS0hADUvZmijCeTdIbm1PvgQGcGrlEDOn3gXA_Vz_Yw";
   HTTPClient http;
   String url = "https://script.google.com/macros/s/" + SCRIPT_ID + "/exec?"
                "temp=" + String(temp) +
                "&pres=" + String(pres) +
                "&hum=" + String(hum) +
                "&gas=" + String(gas) +
                "&co=" + String(co) +
                "&no2=" + String(no2) +
                "&nh3=" + String(nh3) +
                "&so2=" + String(so2) +
                "&h2=" + String(h2) +
                "&lpg=" + String(lpg) +
                "&ch4=" + String(ch4) +
                "&rain=" + String(rain);
   http.begin(url.c_str());
   http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);        
   int httpCode = http.GET();  
 
   String payload;
   http.end();
}

