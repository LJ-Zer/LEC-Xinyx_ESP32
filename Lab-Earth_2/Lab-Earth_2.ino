#include <SPI.h>
#include <LoRa.h>
#include <HTTPUpdate.h>
#include <HTTPClient.h>
#include <WiFi.h>

// Pin definitions for LoRa module
#define SS 5
#define RST 14
#define DI0 2

char ssid[] = "Smart_Bro_0E109";
char pass[] = "DiosteBendiga08";

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
    // For example, if the format is "T:XX.XXC,P:XX.XXhPa,H:XX.XX%,G:XX.XXKOhms,CO:XX,NO2:XX,NH3:XX,MQ4:XX,MQ6:XX,MQ8:XX,MQ136:XX,Rain:XX"
    float temp, pres, hum, gas;
    int co, no2, nh3, mq4, mq6, mq8, mq136, rain;
    sscanf(receivedData.c_str(), 
           "T:%fC,P:%fhPa,H:%f%%,G:%fKOhms,CO:%d,NO2:%d,NH3:%d,MQ4:%d,MQ6:%d,MQ8:%d,MQ136:%d,Rain:%d",
           &temp, &pres, &hum, &gas, &co, &no2, &nh3, &mq4, &mq6, &mq8, &mq136, &rain);

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
    Serial.print(mq136);
    Serial.print("\tRain Sensor Value: ");
    Serial.println(rain);

    Serial.println(); // Add an empty line for readability

    // Send data to Google Sheets
    Sheet(temp, pres, hum, gas, co, no2, nh3, mq4, mq6, mq8, mq136, rain);
  }
}

void Sheet(float temp, float pres, float hum, float gas, int co, int no2, int nh3, int mq4, int mq6, int mq8, int mq136, int rain) 
{
   String SCRIPT_ID = "AKfycbzv_1LlkIUOZxJDe1MMq1h0Df91PSA-OJJKOpHUFoOKunFzh_iyvAe3-0mJG3is8i6lzQ";
   HTTPClient http;
   String url = "https://script.google.com/macros/s/" + SCRIPT_ID + "/exec?"
                "temp=" + String(temp) +
                "&pres=" + String(pres) +
                "&hum=" + String(hum) +
                "&gas=" + String(gas) +
                "&co=" + String(co) +
                "&no2=" + String(no2) +
                "&nh3=" + String(nh3) +
                "&mq4=" + String(mq4) +
                "&mq6=" + String(mq6) +
                "&mq8=" + String(mq8) +
                "&mq136=" + String(mq136) +
                "&rain=" + String(rain);
   http.begin(url.c_str());
   http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);        
   int httpCode = http.GET();  
 
   String payload;
   http.end();
}
