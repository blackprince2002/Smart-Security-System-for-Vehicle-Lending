#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <TinyGPS++.h>
#include <DHT.h>

TinyGPSPlus gps;

// Fingerprint sensor configuration
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial fingerprint_serial(2, 3);
#else
#define fingerprint_serial Serial2
#endif
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerprint_serial);

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);

// Define the ins for buzzer and sensors
const int touchPin = 4;
const int buzzerPin = 10;
const int ledPin = 12;
const int alcohol_sensor_pin = A0;
const int flame_sensor_pin = A1;
const int rain_sensor_pin = 11;

DHT dht(9, DHT11);

// To track if any sensor data is being displayed
bool system_on = false;
bool sensors_on = false;  

bool sensor_on_alcohol = true;
bool sensor_on_flame = true;
bool sensor_on_temperature = true;
bool sensor_on_rain = true;

// Define threshold values for each sensor
const int alcohol_threshold = 700;     
const int flame_threshold = 200;       
const int rain_sensor_threshold = 1;   
const int temperature_threshold = 50;  

// Motor control pins and settings
int mainMotorIN1 = 24;
int mainMotorIN2 = 25;
int wiperMotorIN3 = 26;
int wiperMotorIN4 = 27;

// GSM module configuration
SoftwareSerial gsm_serial(7, 8);

int alcohol_value = 0;
int flame_value = 0;
bool is_raining = false;

void setup() {
  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  pinMode(touchPin, INPUT_PULLUP);

  powerOnSystem();  // Initially, turn on the system

  // Initialize the GSM module
  gsm_serial.begin(9600);
  delay(1000);
  gsm_serial.println("AT+CMGF=1");  // Set SMS text mode
  delay(100);
  gsm_serial.println("AT+CNMI=1,2,0,0,0");  // New SMS notification settings
  delay(100);

  // Initialize the Motor module
  pinMode(mainMotorIN1, OUTPUT);
  pinMode(mainMotorIN2, OUTPUT);
  pinMode(wiperMotorIN3, OUTPUT);
  pinMode(wiperMotorIN4, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);

  // Initialize the fingerprint sensor
  finger.begin(57600);
  dht.begin();
}

void loop() {
  int touchState = digitalRead(touchPin);

  if (touchState == HIGH) {
    if (system_on) {
      // If the system is on, turn it off
      powerOffSystem();
    } else {
      // If the system is off, turn it on
      powerOnSystem();
    }

    delay(2000);
  }

  // Check if the system is on before reading and displaying sensor data
  if (system_on) {
    sensors_on = true;  // Sensors are active

    // Create a string to accumulate sensor data
    String sensorData = "";

    // Read and accumulate sensor data if their respective sensors are turned on
    if (sensor_on_alcohol) {
      int alcohol_value = analogRead(alcohol_sensor_pin);
      sensorData += "Alcohol: " + String(alcohol_value) + "  ";
      checkAndSendAlert("Alcohol", alcohol_value, alcohol_threshold, 0, 0);  // Check and send an alert if needed
    }
 
    if (sensor_on_flame) {
      int flame_value = analogRead(flame_sensor_pin);
      bool isFlameDetected = (flame_value < flame_threshold);
      sensorData += "\n\nFlame Detected: " + String(isFlameDetected ? "Yes" : "No") + "  ";
      if (isFlameDetected) {
        char alertMessage[100];
        snprintf(alertMessage, sizeof(alertMessage), "Alert: Flame detected\nGPS Location: http://maps.google.com/maps?q=%.6f,%.6f", gps.location.lat(), gps.location.lng());
        sendSMS(alertMessage);
      }
    }

    if (sensor_on_temperature) {
      float temperature_value = dht.readTemperature();
      sensorData += "\n\nTemperature: " + String(temperature_value) + "C  ";
      checkAndSendAlert("Temperature", temperature_value, temperature_threshold, 0, 0);  
    }

    if (sensor_on_rain) {
      bool is_raining = digitalRead(rain_sensor_pin) == LOW;
      sensorData += "\n\nRain: " + String(is_raining ? "Yes" : "No") + "  ";
      if (is_raining) {
        sendSMS("Alert: Rain detected");
      }
    }

    displaySensorData(sensorData);
  } 
  else if (sensors_on) {
    // If the system is off, but sensors were active, clear the display
    display.clearDisplay();
    display.display();
    sensors_on = false;  // Sensors are no longer active
  }

  // Check if any alert condition is met and reduce main motor speed if needed
  if (sensor_on_alcohol && (analogRead(alcohol_sensor_pin) > alcohol_threshold)) {
    analogWrite(2, 50);
    digitalWrite(mainMotorIN1, HIGH);
    digitalWrite(mainMotorIN2, LOW);
  } else if (sensor_on_flame && (analogRead(flame_sensor_pin) < flame_threshold)) {
    analogWrite(2, 50);
    digitalWrite(mainMotorIN1, HIGH);
    digitalWrite(mainMotorIN2, LOW);
  } else if (sensor_on_rain && digitalRead(rain_sensor_pin) == LOW) {
    analogWrite(3, 255);
    digitalWrite(wiperMotorIN3, LOW);
    digitalWrite(wiperMotorIN4, HIGH);
    analogWrite(2, 50);
    digitalWrite(mainMotorIN1, HIGH);
    digitalWrite(mainMotorIN2, LOW);
  } else {
    digitalWrite(wiperMotorIN3, LOW);
    digitalWrite(wiperMotorIN4, LOW);
    analogWrite(2, 255);
    digitalWrite(mainMotorIN1, HIGH);
    digitalWrite(mainMotorIN2, LOW);
  }
  // Check for GPS data
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }
  //Latitude
  Serial.print("Latitude: ");
  Serial.println(gps.location.lat(), 6);

  //Longitude
  Serial.print("Longitude: ");
  Serial.println(gps.location.lng(), 6);

  // Check if GPS data is valid
  if (gps.location.isValid()) {
    // Pass the latitude and longitude to the alert function
    checkAndSendAlert("GPS Location", 0, 0, gps.location.lat(), gps.location.lng());
  }
}

void powerOnSystem() {
  system_on = true;
  powerOnSound();  // Play power-on sound
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("System ON");
  display.display();
}

void powerOffSystem() {
  powerOffSound();  // Play power-off sound
                   
  // Display a message and wait for fingerprint authentication
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 5);
  display.println("System OFF     \n\nPlace your finger");
  display.display();

  // Wait for fingerprint authentication
  bool authenticated = false;
  unsigned long authenStartTime = millis();

  while (true) {
    int fingerprint_result = finger.getImage();
    if (fingerprint_result == FINGERPRINT_OK) {
      fingerprint_result = finger.image2Tz();
      if (fingerprint_result == FINGERPRINT_OK) {
        fingerprint_result = finger.fingerFastSearch();
        if (fingerprint_result == FINGERPRINT_OK) {
          // Fingerprint authenticated, turn off the system
          system_on = false;
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(5, 5);
          display.println("System OFF");
          display.display();
          delay(2000);
          break;
        }
      }
    }
    // Check if a timeout period has elapsed 10 seconds without authentication
    if (millis() - authenStartTime >= 10000) {
      // Fingerprint authentication timed out, automatically turn the system back on
      powerOnSystem();
      break;
    }
    delay(500);  // Delay and retry fingerprint scanning
  }
}
// Function to display sensor data on the OLED screen
void displaySensorData(String data) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(data);
  display.display();
}

// Function to check sensor values and send an alert if needed
void checkAndSendAlert(const char* sensorName, int value, int threshold, float latitude, float longitude) {
  // If the condition is met, send an SMS alert
  if (value > threshold) {
    char alertMessage[100];
    snprintf(alertMessage, sizeof(alertMessage), "Alert: %s value exceeded threshold (%d)\nGPS Location: http://maps.google.com/maps?q=%.6f,%.6f", sensorName, value, latitude, longitude);
    sendSMS(alertMessage);
  }
}

// Function to send an SMS message
void sendSMS(const char* message) {
  Serial.println("Sending Message");
  gsm_serial.println("AT+CMGF=1");                   //Sets the GSM Module in Text Mode
  delay(1000);                                      
  gsm_serial.println("AT+CMGS=\"+94779913782\"\r");  
  delay(1000);
  gsm_serial.println(message);  
  Serial.println(message);
  delay(100);
  gsm_serial.println((char)26);  // ASCII code of CTRL+Z
  delay(1000);
}


void powerOnSound() {

  tone(buzzerPin, 10000, 1000);  // Play a 10000Hz tone for 1 second
  delay(1000);
  tone(ledPin, 10000, 1000);  // Play a 10000Hz tone for 1 second
  delay(1000);                   // Delay for 1 second
}


void powerOffSound() {
  tone(buzzerPin, 10000, 1000);  // Play a 20000Hz tone for 1 second
  delay(500);
  tone(ledPin, 10000, 1000);  // Play a 20000Hz tone for 1 second
  delay(500);                     // Delay for 0.5 second

  noTone(buzzerPin);            
  delay(100);
  noTone(ledPin);            
  delay(100);

  tone(buzzerPin, 10000, 1000);  
  delay(500);
  tone(ledPin, 10000, 1000);  
  delay(500);                      
}

