/*
ICM-20948
 
 Circuit:
 ICM20948 sensor attached to pins 10, 11, 12, 13:
 CSB: pin 10
 MOSI: pin 11
 MISO: pin 12
 SCK: pin 13

 */

// the sensor communicates using SPI, so include the library:
#include <SPI.h>
#include <ICM20948.h>


//Sensor's memory register addresses:
const byte READ = 0b10000000;     // ICM20948's read command
const byte WRITE = 0b01111111;   // SCP1000's write command

// pins used for the connection with the sensor
// the others we need are controlled by the SPI library):
const int chipSelectPin = 10;
const int outputEnablePin = 15;

ICM20948 IMU(SPI, chipSelectPin);
const byte WHOAMI = IMU.WHO_AM_I;
byte TEST = 0x07;

//SPI Settings
const int clockSpeed = 500000; //in Hz

void setup() {
  //Start the serial output
  Serial.begin(38400);

  // initalize the chip select pin:
  pinMode(chipSelectPin, OUTPUT);
  pinMode(outputEnablePin, OUTPUT);
  
  digitalWrite(chipSelectPin,HIGH);
  //Enable the logic level shifter
  digitalWrite(outputEnablePin,LOW);

  // start the SPI library:
  SPI.begin();
  
  //TODO: Configure ICM20948:
  byte whoAmI = readByteFromRegister(WHOAMI);
  Serial.println(whoAmI, BIN);
  if (whoAmI != 0xEA) {
    Serial.println("IMU NOT RECOGNIZED");
  }
  byte usrBank = 0x02 << 4;
  writeByteToRegister(0x7F, usrBank); //switch to user bank 2
  byte selfTest = 0b00111000;
  byte gyro1 = 0x00;
  writeByteToRegister(IMU.GYRO_CONFIG_2, selfTest); //enable gyro self 
  writeByteToRegister(IMU.GYRO_CONFIG_1, gyro1); //disable gyro DLPF
  // give the sensor time to set up:
  delay(500);
  Serial.println("Setup compelete");
}

void loop() {
  Serial.println("Starting test cycle");
  

  Serial.print("User Bank (--0123----):");
  byte data = readByteFromRegister(0x7F);
  Serial.println(data,BIN);
  for (int i = 0; i<128; i++){
    Serial.print(i);
    Serial.print(": ");
    TEST = i;
    data = readByteFromRegister(TEST);
    Serial.println(data, BIN);
  }
  delay(500);
}

//Read from or write to register from the ICM20948:
byte readByteFromRegister(byte thisRegister) {
  byte inByte = 0;           // incoming byte from the SPI
  // ICM20948 expects the register name in the lower 7 bits
  // of the byte.
  // now combine the address and the command into one byte
  byte dataToSend = thisRegister | READ;
  // gain control of the SPI bus
  // and configure settings
  SPI.beginTransaction(SPISettings(clockSpeed, MSBFIRST, SPI_MODE3)); //mode based on datahseet sec 6.5 and https://www.arduino.cc/en/Reference/SPI idk if this is right
  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);
  // send the device the register you want to read:
  SPI.transfer(dataToSend);
  // send a value of 0 to read the first byte returned:
  inByte = SPI.transfer(0x00);
  // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
  // release control of the SPI port
  SPI.endTransaction();
  // return the result:
  return(inByte);
}

//Writes the given byte to the given register
void writeByteToRegister(byte thisRegister, byte dataToSend) {
  thisRegister = thisRegister & WRITE;
  SPI.beginTransaction(SPISettings(clockSpeed, MSBFIRST, SPI_MODE3));
  // take the chip select low to select the device:
  digitalWrite(chipSelectPin, LOW);
  // send the device the register you want to write to, then the data:
  SPI.transfer(thisRegister);
  SPI.transfer(dataToSend);
    // take the chip select high to de-select:
  digitalWrite(chipSelectPin, HIGH);
  // release control of the SPI port
  SPI.endTransaction();
}
