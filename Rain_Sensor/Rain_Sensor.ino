
#define RAIN_SENSOR_PIN 13

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Set the rain sensor pin as an input
  pinMode(RAIN_SENSOR_PIN, INPUT);
}

void loop() {
  // Read the digital output from the rain sensor
  int rainDetected = digitalRead(RAIN_SENSOR_PIN);

  // Check if rain is detected
  if (rainDetected == HIGH) {
    Serial.println("Rain detected!");
  } else {
    Serial.println("No rain detected.");
  }

  // Wait before checking again
  delay(1000);
}
