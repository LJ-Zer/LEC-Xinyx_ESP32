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
    // For example, if the format is "T:XX.XXC,P:XX.XXhPa,H:XX.XX%,G:XX.XXKOhms":
    float temp, pres, hum, gas;
    sscanf(receivedData.c_str(), "T:%fC,P:%fhPa,H:%f%%,G:%fKOhms", &temp, &pres, &hum, &gas);

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

    Serial.println(); // Add an empty line for readability

    Sheet(temp, pres, hum, gas);
  }
  
}

void Sheet(float temp, float hum, float pres, float gas) 
{
   String SCRIPT_ID = "AKfycbzv_1LlkIUOZxJDe1MMq1h0Df91PSA-OJJKOpHUFoOKunFzh_iyvAe3-0mJG3is8i6lzQ";
   HTTPClient http;
   String url="https://script.google.com/macros/s/"+SCRIPT_ID+"/exec?temp="+String(temp)+"&pres="+String(pres)+"&hum="+String(hum)+"&gas="+String(gas);
   http.begin(url.c_str());
   http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);        
   int httpCode = http.GET();  
 
   String payload;
   http.end();
}
