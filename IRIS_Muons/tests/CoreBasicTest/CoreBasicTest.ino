/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  This example code is in the public domain.
 */

#include <Wire.h>

// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 13;
int led_r = 30;
int led_g = 31;
int led_y = 32;

// ADXL accelerometer
#define HG_INT1 36
#define HG_INT2 35
#define SCLK_1 27
#define MOSI_1 26
#define MISO_1 39
#define CS_HG 38

// the setup routine runs once when you press reset:
void setup() {
  // initialize the digital pin as an output.
  //Wire1.setSCL(16);
  //Wire1.setSDA(17);
  Wire1.begin();
  Serial.begin(9600);
  pinMode(led, OUTPUT);  

  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(led_y, OUTPUT);  
  digitalWrite(led_r, HIGH);
  digitalWrite(led_g, HIGH);
  digitalWrite(led_y, HIGH);
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(led_r, LOW);
  delay(500);
  digitalWrite(led_g, LOW);
  delay(500);
  digitalWrite(led_y, LOW);
  delay(1000);              
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(led_r, HIGH);
  digitalWrite(led_g, HIGH);
  digitalWrite(led_y, HIGH);
  delay(2000);               // wait for 2 seconds
  scan(); //scan for I2C devices
  delay(1000); 
}

void scan() {
  byte error, address;
  int nDevices;

  Serial.println(F("Scanning..."));

  nDevices = 0;
  for (address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire1.beginTransmission(address);
    error = Wire1.endTransmission();

    if (error == 0) {
      Serial.print(F("Device found at address 0x"));
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address,HEX);
      Serial.print("  (");
      Serial.println(")");

      nDevices++;
    } else if (error==4) {
      Serial.print(F("Unknown error at address 0x"));
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println(F("No I2C devices found\n"));
  } else {
    Serial.println(F("done\n"));
  }
}
