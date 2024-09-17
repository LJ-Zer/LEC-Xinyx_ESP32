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

    // Optionally, parse the data if it's in a known format
    // For example, if the format is "T:XX.XXC,P:XX.XXhPa,H:XX.XX%,G:XX.XXKOhms,CO:XX,NO2:XX,NH3:XX,SO2:XX,H2:XX,LPG:XX,CH4:XX,Rain:XX"
    float temp, pres, hum, gas, vIN, drctn, windSpeed;
    int co, no2, nh3, mq4, mq6, mq8, mq136, rainValue;
    sscanf(receivedData.c_str(), 
           "T:%fC,P:%fhPa,H:%f%%,G:%fKOhms,SVolts:%f,DrctnVolts:%f,WS%f,CO:%d,NO2:%d,NH3:%d,SO2:%d,H2:%d,LPG:%d,CH4:%d,Rain:%d",
           &temp, &pres, &hum, &gas, &vIN, &drctn, &windSpeed,&co, &no2, &nh3, &mq4, &mq6, &mq8, &mq136, &rainValue);

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
    Serial.print(mq136);
    Serial.print("\tH2 Concentration: ");
    Serial.print(mq8);
    Serial.print("\tLPG Concentration: ");
    Serial.print(mq6);
    Serial.print("\tMethane (CH4) Concentration: ");
    Serial.print(mq4);
    Serial.print("\tRain Sensor Value: ");
    Serial.print(rainValue);
    Serial.print("\tSVolts: ");
    Serial.print(vIN);
    Serial.print("\tDrctnVolts: ");
    Serial.print(drctn);
    Serial.print("\t Winspeed");
    Serial.println(windSpeed);

    Serial.println(); // Add an empty line for readability

    // Send data to Google Sheets
    Sheet(temp, pres, hum, gas, co, no2, nh3, mq4, mq6, mq8, mq136, rainValue, vIN, drctn, windSpeed);
  }
}

void Sheet(float temp, float pres, float hum, float gas, float vIN, float windSpeed, int co, int no2, int nh3, int mq4, int mq6, int mq8, int mq136, int rainValue, float drctn) 
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
