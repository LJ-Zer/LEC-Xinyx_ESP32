// Define pins for MiCS-6814 sensors
#define CO_PIN 34      // CO (MiCS-6814)
#define NO2_PIN 35     // NO2 (MiCS-6814)
#define NH3_PIN 32     // NH3 (MiCS-6814)

// Define pins for MQ sensors
#define MQ4_PIN 33     // MQ-4
#define MQ6_PIN 25     // MQ-6
#define MQ8_PIN 26     // MQ-8
#define MQ136_PIN 27   // MQ-136

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
}

void loop() {
  // Read analog values from the MiCS-6814 sensor pins (integer values)
  int co= analogRead(CO_PIN);
  int no2= analogRead(NO2_PIN);
  int nh3= analogRead(NH3_PIN);
  
  // Read analog values from the MQ sensors (integer values)
  int mq4 = analogRead(MQ4_PIN);
  int mq6 = analogRead(MQ6_PIN);
  int mq8 = analogRead(MQ8_PIN);
  int mq136 = analogRead(MQ136_PIN);

  // Print the values to the serial monitor
  Serial.print("CO Value: ");
  Serial.print(co);
  Serial.print("\tNO2 Value: ");
  Serial.print(no2);
  Serial.print("\tNH3 Value: ");
  Serial.print(nh3);
  
  Serial.print("\tMQ-4 Value: ");
  Serial.print(mq4);
  Serial.print("\tMQ-6 Value: ");
  Serial.print(mq6);
  Serial.print("\tMQ-8 Value: ");
  Serial.print(mq8);
  Serial.print("\tMQ-136 Value: ");
  Serial.println(mq136);
  
  // Add a delay to prevent flooding the serial monitor
  delay(1000);
}
