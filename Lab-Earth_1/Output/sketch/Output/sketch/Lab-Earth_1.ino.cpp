#line 1 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Output\\sketch\\Lab-Earth_1.ino.cpp"
#include <Arduino.h>
#line 1 "C:\\Users\\John Buenaflor\\Git_Repo\\LEC-Xinyx\\Lab-Earth_1\\Lab-Earth_1.ino"
#include <HTTPUpdate.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <IRremote.h>
#include <WiFi.h>
#include <esp_system.h>  // Include the necessary library for ESP.restart()
#include <ESP32Servo.h>

const int servoPin = 25;
Servo myServo;

#define IR_RECEIVE_PIN 4
#define PIRPIN 15       
#define MOTOR_IN3 27   
#define MOTOR_IN4 26    
#define MOTOR_ENB 14    
const int photoResistorPin = 34;

Adafruit_BME280 bme;
LiquidCrystal_I2C lcd(0x27, 20, 4);

unsigned long delayTime;
long randNumber;
volatile bool restartRequested = false;

int bulb1 = 0;
int bulb2 = 0;
int bulb3 = 0;
int states = 0;

String payload;

char ssid[] = "Smart_Bro_0E109";
char pass[] = "DiosteBendiga08";

// Global state
enum Mode { MODE_ECO, MODE_STANDARD, MODE_STUDY, MODE_ULIT, MODE_UP, MODE_DOWN, MODE_NONE };
Mode currentMode = MODE_NONE;

// Task Handles
TaskHandle_t irTaskHandle;
TaskHandle_t modeTaskHandle;

void IRCommandTask(void *parameter) {
  while (true) {
    if (IrReceiver.decode()) {
      IrReceiver.resume();
      int remote = IrReceiver.decodedIRData.command;

      // Notify the mode task with the IR command value
      xTaskNotify(modeTaskHandle, remote, eSetValueWithOverwrite);
    }
    delay(100); // Adjust delay as needed
  }
}

void ModeTask(void *parameter) {
  while (true) {
    // Wait for a notification from the IR task
    uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (restartRequested) {
      ESP.restart();
    }

    // Decide which mode to run based on the notification value
    switch (notifiedValue) {
      case 69:
        currentMode = MODE_ECO;
        break;
      case 70:
        currentMode = MODE_STANDARD;
        break;
      case 71:
        currentMode = MODE_STUDY;
        break;
      case 28:
        currentMode = MODE_ULIT;
        break;
      case 24:
        currentMode = MODE_UP;
        break;
      case 82:
        currentMode = MODE_DOWN;
        break;
      default:
        currentMode = MODE_NONE;
        break;
    }

    delay(1000); // Delay to prevent rapid mode switching
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  IrReceiver.begin(IR_RECEIVE_PIN);

  // Initialize motor pins
  pinMode(PIRPIN, INPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);
  pinMode(MOTOR_ENB, OUTPUT);
  
//  dht.begin();
  
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  
  lcd.begin();
  lcd.backlight();
  myServo.attach(servoPin);
  myServo.write(5);
  myServo.detach();

  // Create tasks
  xTaskCreatePinnedToCore(IRCommandTask, "IRCommandTask", 8192, NULL, 1, &irTaskHandle, 0);
  xTaskCreatePinnedToCore(ModeTask, "ModeTask", 8192, NULL, 1, &modeTaskHandle, 1);
}

void loop() {
  // Main loop handles mode execution
  switch (currentMode) {
    case MODE_ECO:
      eco();
      break;
    case MODE_STANDARD:
      standard();
      break;
    case MODE_STUDY:
      study();
      break;
    case MODE_ULIT:
      ulit();
      break;
    case MODE_UP:
      up();
      break;
    case MODE_DOWN:
      down();
      break;
    default:
      // No mode selected, or invalid mode
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("Select Mode"); 
      lcd.setCursor(0, 1);
      lcd.print("1. Eco Mode");
      lcd.setCursor(0, 2);
      lcd.print("2. Standard Mode");
      lcd.setCursor(0, 3);
      lcd.print("3. Study Mode");
      delay (1000);
      break;
  }
  if (restartRequested) {
    Serial.println("Restart requested. Restarting ESP32...");
    ESP.restart();
  }
}

void eco() {
  Serial.println("Eco Mode");
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("Eco Mode");
  lcd.setCursor(7, 2);
  lcd.print("ENABLE");
  delay(2000);
  
  myServo.attach(servoPin);
  delay(500);
  myServo.write(110);
  Serial.println("up");
  delay(1000);
  
  // Update every second
  while (currentMode == MODE_ECO) {
    if (restartRequested) {
      ESP.restart();
    }

    delay(delayTime);
    int pirState = digitalRead(PIRPIN);
    float Temperature = bme.readTemperature();
    float Humidity = bme.readHumidity() - 10;
    float lightValue = analogRead(photoResistorPin);

    Serial.print("Light: ");
    Serial.println(lightValue);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(Temperature);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(Humidity);
    lcd.print(" %");

    int motorSpeed = 0;

    if (pirState == HIGH) {
      if (Temperature < 15.0) {
        analogWrite(MOTOR_ENB, 0);
        digitalWrite(MOTOR_IN3, LOW);
        digitalWrite(MOTOR_IN4, LOW);
        motorSpeed = 0;
        Serial.println("Motion detected and temperature < 15°C. Fan off.");
      } else if (Temperature >= 15.0 && Temperature < 20.0) {
        digitalWrite(MOTOR_IN3, LOW);
        digitalWrite(MOTOR_IN4, LOW);
        analogWrite(MOTOR_ENB, 100);
        motorSpeed = 1;
        Serial.println("Motion detected and temperature 15°C to 20°C. Fan at speed 1.");
      } else if (Temperature >= 20.0 && Temperature < 25.0) {
        digitalWrite(MOTOR_IN3, LOW);
        digitalWrite(MOTOR_IN4, LOW);
        analogWrite(MOTOR_ENB, 100);
        motorSpeed = 1;
        Serial.println("Motion detected and temperature 20°C to 25°C. Fan at speed 2.");
      } else if (Temperature >= 25.0) {
        digitalWrite(MOTOR_IN3, HIGH);
        digitalWrite(MOTOR_IN4, LOW);
        analogWrite(MOTOR_ENB, 100);
        motorSpeed = 1;
        Serial.println("Motion detected and temperature > 25°C. Fan at maximum speed.");
      }
      lcd.setCursor(0, 2);
      lcd.print("Motion: Detected");
      states = 1;
      
      if (lightValue > 2730) {
        lcd.setCursor(0, 3);
        lcd.print("LS 1:OFF 2:OFF 3:OFF");
        bulb1 = 0;
        bulb2 = 0;
        bulb3 = 0;
        states = 1;
      } else if (lightValue > 1365) {
        lcd.setCursor(0, 3);
        lcd.print("LS 1:OFF 2:OFF 3:OFF");
        bulb1 = 0;
        bulb2 = 0;
        bulb3 = 0;
        states = 1;
      } else {
        lcd.setCursor(0, 3);
        lcd.print("LS 1:ON 2:OFF 3:OFF");
        bulb1 = 0;
        bulb2 = 0;
        bulb3 = 0;
        states = 1;        
      }
    } 
    else {
      analogWrite(MOTOR_ENB, 0);
      digitalWrite(MOTOR_IN3, LOW);
      digitalWrite(MOTOR_IN4, LOW);
      motorSpeed = 0;
      Serial.println("No motion detected. Fan off.");
      lcd.setCursor(0, 2);
      lcd.print("Motion: None");
      lcd.setCursor(0, 3);
      lcd.print("LS 1:OFF 2:OFF 3:OFF");
      bulb1 = 0;
      bulb2 = 0;
      bulb3 = 0;  
      states = 1;    
    }

    write_to_google_sheet(Temperature, Humidity, lightValue, pirState, motorSpeed, bulb1, bulb2, bulb3, states);

    delay(1000); // Adjust delay to control how often the mode updates
  }
}

void study() {
  Serial.println("Study Mode");
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Study Mode");
  lcd.setCursor(7, 2);
  lcd.print("ENABLE");
  delay(2000);
  // Update every second
  while (currentMode == MODE_STUDY) {
    if (restartRequested) {
      ESP.restart();
    }

    delay(delayTime);
    
    int pirState = digitalRead(PIRPIN);
    float Temperature = bme.readTemperature();
    float Humidity = bme.readHumidity() - 10;
    float lightValue = analogRead(photoResistorPin);
    Serial.print("Light: ");
    Serial.println(lightValue);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(Temperature);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(Humidity);
    lcd.print(" %");

    int motorSpeed = 0;

    if (pirState == HIGH) {
      if (Temperature < 15.0) {
        analogWrite(MOTOR_ENB, 0);
        digitalWrite(MOTOR_IN3, LOW);
        digitalWrite(MOTOR_IN4, LOW);
        motorSpeed = 0;
        Serial.println("Motion detected and temperature < 15°C. Fan off.");
      } else if (Temperature >= 15.0 && Temperature < 20.0) {
        digitalWrite(MOTOR_IN3, HIGH);
        digitalWrite(MOTOR_IN4, LOW);
        analogWrite(MOTOR_ENB, 85);
        motorSpeed = 1;
        Serial.println("Motion detected and temperature 15°C to 20°C. Fan at speed 1.");
      } else if (Temperature >= 20.0 && Temperature < 25.0) {
        digitalWrite(MOTOR_IN3, HIGH);
        digitalWrite(MOTOR_IN4, LOW);
        analogWrite(MOTOR_ENB, 170);
        motorSpeed = 2;
        Serial.println("Motion detected and temperature 20°C to 25°C. Fan at speed 2.");
      } else if (Temperature >= 25.0) {
        digitalWrite(MOTOR_IN3, HIGH);
        digitalWrite(MOTOR_IN4, LOW);
        analogWrite(MOTOR_ENB, 255);
        motorSpeed = 3;
        Serial.println("Motion detected and temperature > 25°C. Fan at maximum speed.");
      }
      lcd.setCursor(0, 2);
      lcd.print("Motion: Detected");
      states = 3;
      
      if (lightValue > 2730) {
        lcd.setCursor(0, 3);
        lcd.print("LS 1:ON 2:ON 3:ON");
        bulb1 = 1;
        bulb2 = 1;
        bulb3 = 1;
        states = 3;
      } else if (lightValue > 1365) {
        lcd.setCursor(0, 3);
        lcd.print("LS 1:ON 2:ON 3:ON");
        bulb1 = 1;
        bulb2 = 1;
        bulb3 = 1;
        states = 3;
      } else {
        lcd.setCursor(0, 3);
        lcd.print("LS 1:ON 2:ON 3:ON");
        bulb1 = 1;
        bulb2 = 1;
        bulb3 = 1;
        states = 3;        
      }
    } 
    else {
      analogWrite(MOTOR_ENB, 0);
      digitalWrite(MOTOR_IN3, LOW);
      digitalWrite(MOTOR_IN4, LOW);
      motorSpeed = 0;
      Serial.println("No motion detected. Fan off.");
      lcd.setCursor(0, 2);
      lcd.print("Motion: None");
      lcd.setCursor(0, 3);
      lcd.print("LS 1:OFF 2:OFF 3:OFF");
      bulb1 = 0;
      bulb2 = 0;
      bulb3 = 0;
      states = 3;
    }

    write_to_google_sheet(Temperature, Humidity, lightValue, pirState, motorSpeed, bulb1, bulb2, bulb3, states);

    delay(1000); // Adjust delay to control how often the mode updates
  }
}

void standard() {
  Serial.println("Standard Mode");
  lcd.clear();
  lcd.setCursor(3, 0);
  lcd.print("Standard Mode");
  lcd.setCursor(7, 2);
  lcd.print("ENABLE");
  delay(2000);
  
  // Update every second
  while (currentMode == MODE_STANDARD) {
    if (restartRequested) {
      ESP.restart();
    }

    delay(delayTime);
    
    int pirState = digitalRead(PIRPIN);
    float Temperature = bme.readTemperature();
    float Humidity = bme.readHumidity() - 10;
    float lightValue = analogRead(photoResistorPin);

    Serial.print("Light: ");
    Serial.println(lightValue);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(Temperature);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: ");
    lcd.print(Humidity);
    lcd.print(" %");

    int motorSpeed = 0;

    if (pirState == HIGH) {
      if (Temperature < 15.0) {
        analogWrite(MOTOR_ENB, 0);
        digitalWrite(MOTOR_IN3, LOW);
        digitalWrite(MOTOR_IN4, LOW);
        motorSpeed = 0;
        Serial.println("Motion detected and temperature < 15°C. Fan off.");
      } else if (Temperature >= 15.0 && Temperature < 20.0) {
        digitalWrite(MOTOR_IN3, HIGH);
        digitalWrite(MOTOR_IN4, LOW);
        analogWrite(MOTOR_ENB, 85);
        motorSpeed = 1;
        Serial.println("Motion detected and temperature 15°C to 20°C. Fan at speed 1.");
      } else if (Temperature >= 20.0 && Temperature < 25.0) {
        digitalWrite(MOTOR_IN3, HIGH);
        digitalWrite(MOTOR_IN4, LOW);
        analogWrite(MOTOR_ENB, 170);
        motorSpeed = 2;
        Serial.println("Motion detected and temperature 20°C to 25°C. Fan at speed 2.");
      } else if (Temperature >= 25.0) {
        digitalWrite(MOTOR_IN3, HIGH);
        digitalWrite(MOTOR_IN4, LOW);
        analogWrite(MOTOR_ENB, 255);
        motorSpeed = 3;
        Serial.println("Motion detected and temperature > 25°C. Fan at maximum speed.");
      }
      lcd.setCursor(0, 2);
      lcd.print("Motion: Detected");
      states = 2;

     if (lightValue <= 1300) {
        lcd.setCursor(0, 3);
        lcd.print("LS 1:ON 2:ON 3:ON");
        bulb1 = 1;
        bulb2 = 1;
        bulb3 = 1;
        states = 2;
      } else if (lightValue >= 1400 && lightValue <= 2600) {
        lcd.setCursor(0, 3);
        lcd.print("LS 1:ON 2:ON 3:OFF");
        bulb1 = 1;
        bulb2 = 1;
        bulb3 = 0;
        states = 2;
      } else if (lightValue >= 2900 && lightValue <= 3900) {
        lcd.setCursor(0, 3);
        lcd.print("LS 1:ON 2:OFF 3:OFF");
        bulb1 = 1;
        bulb2 = 0;
        bulb3 = 0;
        states = 2;
      } else if (lightValue == 4095) {
        lcd.setCursor(0, 3);
        lcd.print("LS 1:OFF 2:OFF 3:OFF");
        bulb1 = 0;
        bulb2 = 0;
        bulb3 = 0;
        states = 2;
      }
    } 
    else {
      analogWrite(MOTOR_ENB, 0);
      digitalWrite(MOTOR_IN3, LOW);
      digitalWrite(MOTOR_IN4, LOW);
      motorSpeed = 0;
      Serial.println("No motion detected. Fan off.");
      lcd.setCursor(0, 2);
      lcd.print("Motion: None");
      lcd.setCursor(0, 3);
      lcd.print("LS 1:OFF 2:OFF 3:OFF");
      bulb1 = 0;
      bulb2 = 0;
      bulb3 = 0;
      states = 2;
    }

    write_to_google_sheet(Temperature, Humidity, lightValue, pirState, motorSpeed, bulb1, bulb2, bulb3, states);

    delay(1000); // Adjust delay to control how often the mode updates
  }
}

void ulit() {
  Serial.println("Reset Mode");
  lcd.clear();
  lcd.setCursor(7, 2);
  lcd.print("RESET");
  delay(2000);
  restartRequested = true;
}

void up() {
  myServo.attach(servoPin);
  delay(500);
  myServo.write(110);
  myServo.detach();
  Serial.println("up");
}

void down() {
  myServo.attach(servoPin);
  delay(500);
  myServo.write(5);
  myServo.detach();
  Serial.println("down");  
}

void write_to_google_sheet(float Temperature, float Humidity, float lightValue, int pirState, int motorSpeed, int bulb1, int bulb2, int bulb3, int states) {
  String SCRIPT_ID = "AKfycbyDVjLXSJxZDwenz7dOzadMBV_d_zlHHMWPUDhMzBcRvJexphSu8MBKSsDOsy-aKuPm9Q";
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + SCRIPT_ID + "/exec?Temperature=" + String(Temperature) + "&Humidity=" + String(Humidity) + "&lightValue=" + String(lightValue) + "&pirState=" + String(pirState) + "&motorSpeed=" + String(motorSpeed) + "&bulb1=" + String(bulb1) + "&bulb2=" + String(bulb2) + "&bulb3=" + String(bulb3) + "&states=" + String(states);
  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    
    Serial.println("Google Sheets response: " + payload);
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Power Consumption");
    lcd.setCursor(7, 1);
    lcd.print("(kWh)");    
    lcd.setCursor(0, 3);
    lcd.print(payload);
    delay(1000);  
    
  } else {
    Serial.println("Error in HTTP request: " + String(httpCode));
  }
  http.end();
}

