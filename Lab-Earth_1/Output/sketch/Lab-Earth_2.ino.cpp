#include <Arduino.h>
#line 1 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_2\\Lab-Earth_2.ino"
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>  // Needed for HTTP requests
#include <HTTPClient.h>  // Needed for sending data to Google Sheets

#define SS 5
#define RST 14
#define DI0 2

#line 10 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_2\\Lab-Earth_2.ino"
void setup();
#line 35 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_2\\Lab-Earth_2.ino"
void loop();
#line 66 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_2\\Lab-Earth_2.ino"
void parseData(String data, float &temp, float &pres, float &hum, float &gas, int &co, int &no2, int &nh3, int &mq4, int &mq6, int &mq8, int &mq136, int &rainValue, float &vIN, float &drctn, float &windSpeed);
#line 78 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_2\\Lab-Earth_2.ino"
void Sheet(float temp, float pres, float hum, float gas, float vIN, float drctn, float windSpeed, int co, int no2, int nh3, int mq4, int mq6, int mq8, int mq136, int rainValue);
#line 10 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_2\\Lab-Earth_2.ino"
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial);

  // Initialize LoRa
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed. Check your connections.");
    while (1);
  }
  Serial.println("LoRa Initializing OK");

  // Connect to WiFi
  WiFi.begin("PLDTHOMEFIBR53690", "PLDTWIFI4s4F8");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");

  // Set LoRa to receive mode
  LoRa.receive();
}

void loop() {
  // Check if LoRa received a packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet: ");

    // Read the packet content
    String receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    Serial.println(receivedData);

    // Parse the received packet (assuming it's in key-value format like T:25.0C,P:1000.0hPa,...)
    float temp, pres, hum, gas, vIN, drctn, windSpeed;
    int co, no2, nh3, mq4, mq6, mq8, mq136, rainValue;

    // Example function to parse the data string (implement this according to your format)
    parseData(receivedData, temp, pres, hum, gas, co, no2, nh3, mq4, mq6, mq8, mq136, rainValue, vIN, drctn, windSpeed);

    Serial.print("Temp: "); Serial.println(temp);
    Serial.print("Pres: "); Serial.println(pres);
    Serial.print("Hum: "); Serial.println(hum);
// Print other variables similarly


    // Send the parsed data to Google Sheets
    Sheet(temp, pres, hum, gas, vIN, drctn, windSpeed, co, no2, nh3, mq4, mq6, mq8, mq136, rainValue);
  }
}

void parseData(String data, float &temp, float &pres, float &hum, float &gas, int &co, int &no2, int &nh3, int &mq4, int &mq6, int &mq8, int &mq136, int &rainValue, float &vIN, float &drctn, float &windSpeed) {
  // Assuming the data comes in this format: T:25.0C,P:1000.0hPa,H:50.0%,...
  // You can use String manipulation to split and parse the data.
  
  // Example parsing function for extracting each value
  // Implement according to the format of the received data
  Serial.print(temp);

  // Use data.indexOf(), data.substring(), and data.toFloat() to extract values
}

// Function to send data to Google Sheets
void Sheet(float temp, float pres, float hum, float gas, float vIN, float drctn, float windSpeed, int co, int no2, int nh3, int mq4, int mq6, int mq8, int mq136, int rainValue) {
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
               "&mq136=" + String(mq136) +
               "&mq8=" + String(mq8) +
               "&mq6=" + String(mq6) +
               "&mq4=" + String(mq4) +
               "&rainValue=" + String(rainValue) +
               "&vIN=" + String(vIN) +
               "&drctn=" + String(drctn) +
               "&windSpeed=" + String(windSpeed);

  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Response from Google Sheets: " + payload);
  } else {
    Serial.println("Error sending data to Google Sheets");
  }

  http.end();
}

