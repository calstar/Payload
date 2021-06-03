/*
  Tests basic functionality for IRIS Core
  - Blink R, Y, G LEDs in sequence along with Teenst on-board LED
  - Scan for I2C devices and print results to serial
  - Check for ADXL high-g accelerometer on SPI
  - Test read/write to SD card onboard Teensy
 */

#include <Wire.h>
// ADXL accelerometer
#include "ADXL372.h" // Click here to get library: https://github.com/gednrs/ADXL372
#include <SPI.h>

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
#define ADXL372_DEVID                  0x00u   /* Analog Devices, Inc., accelerometer ID */
#define ADXL372_DEVID_VAL             0xADu   /* Analog Devices, Inc., accelerometer ID */
#define ADXL372_RESET                 0x41u   /* Reset */
#define ADXL372_RESET_CODE            0x52u /* Writing code 0x52 resets the device */

/* SPI objects (via hardware) */
ADXL372 myADXL = ADXL372(CS_HG, MOSI_1, MISO_1, SCLK_1);

/* class objects and functions from sensors */
// ADXL
struct ADXL372_AccelTriplet accel;
struct ADXL372_AccelTripletG accelG;

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

  // start the SPI library:
  //SPI1.begin();
  //SPI.setMOSI(MOSI_1);
  //SPI.setMISO(MISO_1);
  //SPI.setSCK(SCLK_1);
  // initalize the chip select pin:
  pinMode(CS_HG, OUTPUT);
  digitalWrite(CS_HG, HIGH);
  
  
  // Attempt to initialize ADXL
  if (myADXL.begin() == ADXL372_DEVID_VAL) {
    Serial.println("ADXL372 found!");
  } else {
    Serial.println("ADXL372 not found!");
  }
  // sets up ADXL
  myADXL.Set_BandWidth(BW_3200Hz);          // bandwidth (frequency response)
  myADXL.Set_low_noise(true);               // low noise mode
  myADXL.Set_ODR(ODR_6400Hz);               // output data rate
  myADXL.Set_hpf_corner(HPF_CORNER0);       // high pass filter cutoff frequency
  myADXL.Set_op_mode(FULL_BW_MEASUREMENT);  // operation mode
  
}

// the loop routine runs over and over again forever:
void loop() {
  blink_sequence(); // Takes 4 seconds to blink LEDs
  scan(); //scan for I2C devices and output result over serial (9600 baud)
  //read_ADXL();
  //ADXLscan();
  delay(200); 
}

void blink_sequence() {
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
  //delay(2000);               // wait for 2 seconds
}


void read_ADXL() {
    // the ADXL values are only updated when you call 'ReadAccTriplet'
    myADXL.ReadAccTriplet(&accel);
    accelG = myADXL.ConvertAccTripletToG(&accel);
    // ADXL values
    Serial.print("ADXL Acc (mg) [ ");
    printFormattedFloat( accelG.x / 1000.0, 5, 2 );
    Serial.print(", ");
    printFormattedFloat( accelG.y / 1000.0, 5, 2 );
    Serial.print(", ");
    printFormattedFloat( accelG.z / 1000.0, 5, 2 );
    Serial.print(" ]");
    Serial.println();
}

void ADXLscan() {
    byte id = readRegister(ADXL372_DEVID, 1);
    Serial.print("Read: ");
    Serial.print(id, HEX);
    Serial.print("; Expected: ");
    Serial.println(ADXL372_DEVID_VAL, HEX);
}

// Scan for I2C devices
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

// Numerically formats data values.
void printFormattedFloat(float val, uint8_t leading, uint8_t decimals){
  float aval = abs(val);
  if(val < 0){
    Serial.print("-");
  }else{
    Serial.print(" ");
  }
  for( uint8_t indi = 0; indi < leading; indi++ ){
    uint32_t tenpow = 0;
    if( indi < (leading-1) ){
      tenpow = 1;
    }
    for(uint8_t c = 0; c < (leading-1-indi); c++){
      tenpow *= 10;
    }
    if( aval < tenpow){
      Serial.print("0");
    }else{
      break;
    }
  }
  if(val < 0){
    Serial.print(-val, decimals);
  }else{
    Serial.print(val, decimals);
  }
}

//Read from or write to register from the ADXL:
unsigned int readRegister(byte thisRegister, int bytesToRead) {
  byte inByte = 0;           // incoming byte from the SPI
  unsigned int result = 0;   // result to return
  Serial.print(thisRegister, BIN);
  Serial.print("\t");

  byte dataToSend = (thisRegister << 1) | 0x01; //ADXL read format from library
  Serial.println(thisRegister, BIN);
  // gain control of the SPI port
  // and configure settings
  SPI1.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  // take the chip select low to select the device:
  digitalWrite(CS_HG, LOW);
  // send the device the register you want to read:
  SPI1.transfer(dataToSend);
  // send a value of 0 to read the first byte returned:
  result = SPI1.transfer(0x00);
  // decrement the number of bytes left to read:
  bytesToRead--;
  // if you still have another byte to read:
  if (bytesToRead > 0) {
    // shift the first byte left, then get the second byte:
    result = result << 8;
    inByte = SPI1.transfer(0x00);
    // combine the byte you just got with the previous one:
    result = result | inByte;
    // decrement the number of bytes left to read:
    bytesToRead--;
  }
  // take the chip select high to de-select:
  digitalWrite(CS_HG, HIGH);
  // release control of the SPI port
  SPI1.endTransaction();
  // return the result:
  return(result);
}
