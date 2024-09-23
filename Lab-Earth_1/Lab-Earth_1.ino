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

    // Parse the data if it's in the known format
    float temp, pres, hum, gas, vIN, drctn, windSpeed;
    int co, no2, nh3, mq4, mq6, mq8, mq136, rainValue;

    sscanf(receivedData.c_str(), 
           "T:%f,P:%f,H:%f,G:%f,CO:%d,NO2:%d,NH3:%d,MQ4:%d,MQ6:%d,MQ8:%d,Svlts:%f,DVlts:%f,WS:%f,MQ136:%d,Rain:%d",
           &temp, &pres, &hum, &gas, &co, &no2, &nh3, &mq4, &mq6, &mq8, &vIN, &drctn, &windSpeed, &mq136, &rainValue);

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
    Serial.println(co);
    
    Serial.print("NO2 Value: ");
    Serial.println(no2);
    
    Serial.print("NH3 Value: ");
    Serial.println(nh3);
    
    Serial.print("MQ-4 Value: ");
    Serial.println(mq4);
    
    Serial.print("MQ-6 Value: ");
    Serial.println(mq6);
    
    Serial.print("MQ-8 Value: ");
    Serial.println(mq8);
    
    Serial.print("MQ-136 Value: ");
    Serial.println(mq136);
    
    Serial.print("Solar Voltage: ");
    Serial.println(vIN);
    
    Serial.print("Wind Direction Voltage: ");
    Serial.println(drctn);
    
    Serial.print("Wind Speed: ");
    Serial.print(windSpeed);
    Serial.println(" m/s");
    
    Serial.print("Rain Sensor Value: ");
    Serial.println(rainValue);

    Serial.println(); // Add an empty line for readability

    // Send data to Google Sheets
    Sheet(temp, pres, hum, gas, co, no2, nh3, mq4, mq6, mq8, mq136, rainValue, vIN, drctn, windSpeed);

  }
}


void Sheet(float temp, float pres, float hum, float gas, float vIN, float windSpeed, int co, int no2, int nh3, int mq4, int mq6, int mq8, int mq136, int rainValue, float drctn) 
{
   String SCRIPT_ID = "AKfycbwGh7xapOrp3nWmB0Ddmhwqt7IW947nhMxUf5-oahwc7HfWFvNk9SfsPNXIjqRkr798og";
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
                "&drtcn=" + String(drctn)+
                "&windSpeed=" + String(windSpeed);

   http.begin(url.c_str());
   http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);        
   int httpCode = http.GET();  
 
   String payload;
   http.end();
}
