#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <SPI.h>


Adafruit_SSD1306 Beas = Adafruit_SSD1306(128,64,&Wire);
const int touchPin = 4; // Pin for the TTP223 touch sensor

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
Beas.begin(SSD1306_SWITCHCAPVCC, 0x3C);

 pinMode(touchPin, INPUT);

 
}

void loop() {
  // put your main code here, to run repeatedly:
int touchPinValue = digitalRead(touchPin);

if (touchPinValue == HIGH)

{

Serial.println("TOUCHED");

Beas.clearDisplay();
Beas.setTextColor(SSD1306_WHITE);
Beas.setTextSize(2);
Beas.setCursor(10, 10);
Beas.print("TOUCHED");
Beas.display();
}
else{

Serial.println("not touched");
Beas.clearDisplay();
Beas.setTextColor(SSD1306_WHITE);
Beas.setTextSize(2);
Beas.setCursor(10, 10);
Beas.print("NO TOUCHE");
Beas.display();

}
}

