#include <Adafruit_Fingerprint.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>


// Fingerprint sensor configuration
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial fingerprint_serial(2, 3);
#else
#define fingerprint_serial Serial2
#endif
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerprint_serial);

void setup(){

 finger.begin(57600); 
}


void loop() {

  // Wait for fingerprint authentication in power off function
  bool authenticated = false;
  unsigned long authenticationStartTime = millis();

  while (!authenticated) {
    int fingerprint_result = finger.getImage();
    if (fingerprint_result == FINGERPRINT_OK) {
      fingerprint_result = finger.image2Tz();
      if (fingerprint_result == FINGERPRINT_OK) {
        fingerprint_result = finger.fingerFastSearch();
        if (fingerprint_result == FINGERPRINT_OK) {
          // Fingerprint authenticated, turn off the system
         // system_on = false;
          authenticated = true;
         /* display.clearDisplay();
          display.setCursor(5, 5);
          display.println("System OFF");
          display.display();
          delay(2000);*/
        }
      }
    }

    // Check if a timeout period has elapsed (e.g., 30 seconds) without authentication
    if (millis() - authenticationStartTime >= 10000) {
      // Fingerprint authentication timed out, automatically turn the system back on
      //powerOnSystem();
      break;
    }

    delay(500); // Delay and retry fingerprint scanning
  }
}
