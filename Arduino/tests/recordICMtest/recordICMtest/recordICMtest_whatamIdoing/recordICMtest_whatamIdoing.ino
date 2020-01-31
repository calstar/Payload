/**
Record IMU data and record to the SD card on a Teensy 3.6 using SDHC (not SPI)
Rajiv Govindjee
ra@berkeley.edu
**/
#include "ICM_20948.h"  // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU

#define SERIAL_PORT Serial
#define SPI_PORT SPI    // Your desired SPI port.       Used only when "USE_SPI" is defined
#define CS_PIN 10        // Which pin you connect CS to. Used only when "USE_SPI" is defined
#define WIRE_PORT Wire  // Your desired Wire port.      Used when "USE_SPI" is not defined
ICM_20948_SPI myICM;  // If using SPI create an ICM_20948_SPI object

#include <SD.h>

File myFile;

const int chipSelect = BUILTIN_SDCARD; // Maps to 254 I think
int counter = 0;

// initializes iterative file naming variables
int filecounter = 1;
char filename[128];

// Returns the number of milliseconds passed since the Arduino board began running the current program.
unsigned int last_time;

// Variable to keep track of whether IRIS should be recording.
int rec_var = 2;
int disp_var = 2;

void setup()
{ 
  last_time = millis();
  snprintf(filename, 128, "FILE-%d.txt", filecounter);
 // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial.println("Opened serial connection.");
  /**while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }**/
  // Start SPI for IMU
  SPI_PORT.begin();
  // Attempt to initialize the IMU
  bool initialized = false;
  while( !initialized ){
    myICM.begin( CS_PIN, SPI_PORT ); 
    SERIAL_PORT.print( F("Initialization of the sensor returned: ") );
    SERIAL_PORT.println( myICM.statusString() );
    if( myICM.status != ICM_20948_Stat_Ok ){
      SERIAL_PORT.println( "Trying again..." );
      delay(500);
    }else{
      initialized = true;
    }
  }
  
  //Attempt to initialize the SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) { // Uses SDHC not SPI
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // Open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  
  // if the filename already exists
  while (SD.exists(filename)) {
    filecounter++;    // increment filecount by one
    snprintf(filename, 128, "FILE-%d.txt", filecounter);
  }

  myFile = SD.open(filename, FILE_WRITE);
  
  // Display the sucess of opening the file
  if (myFile) {
    Serial.println("File successfully opened");
  } else {
    Serial.println("Error opening test.txt");
  }
}

void loop()
{
  //loop for a finite amount of time to avoid having to stop in the middle of a write
//  while (counter < 100) {
//    if (myICM.dataReady()) {
//      myICM.getAGMT();                // The values are only updated when you call 'getAGMT'
//      printScaledAGMT(myICM.agmt);   // This function takes into account the scale settings from when the measurement was made to calculate the values with units
//      delay(30);
//    } else {
//      Serial.println("Waiting for data");
//      delay(500);
//    }
//    counter++;
//  }
//  if (counter == 100) {
//    myFile.close();
//    myFile = SD.open("test.txt");
//    if (myFile) {
//      Serial.println("test.txt:");
//      // read from the file until there's nothing else in it:
//      while (myFile.available()) {
//        Serial.write(myFile.read());
//      }
//      // close the file:
//      myFile.close();
//    } else {
//      // if the file didn't open, print an error:
//      Serial.println("error opening test.txt");
//    }
//    counter++;
//  }
//  myFile.close();
//  Serial.println("Finished loop data");

  // Serial.available gets the number of bytes (characters) available for reading from the serial port.
  // The serial receive buffer can only store up to 64 bytes of data, so the start/stop commands are single characters.
  // To start recording, input "i" into serial monitor. To stop recording, input "o" into serial monitor.
  
  // If there is readable data inputted into the serial port:
  if (Serial.available() > 0) {

      // Serial.read() gets a character from the buffer and then throws it away, so the first one will be gone by the time you get to the elseif.
      // Saves the result of Serial.read() in a variable, then the if/elseif statements check against that variable.
      char ser_var = Serial.read();
      
      // Reads incoming serial data. Sets rec_var to "on" if 'i' is inputted into serial monitor.
      if (ser_var == 'i') {
        disp_var = 1;
        Serial.print("Now recording.");       // status message
        Serial.print('\n');                   // starts a new line
      }
      // Reads incoming serial data. Sets rec_var to "off" if 'o' is inputted into serial monitor.
      // For some reason I can't debug, it'll only stop if you type more than one 'o' into the serial monitor.
      else if (ser_var == 'o') {
        disp_var = 0;
        Serial.print("Recording stopped.");   // status message
        Serial.print('\n');                   // starts a new line.
      }
      else {
      }
  }
  else {
  }

  // Prints out status message to serial monitor indicating whether recording is on.
    if (disp_var == 1) {
      Serial.print("IRIS is recording.");
      Serial.println();
      rec_var = 1;
      disp_var = 3;
  } else if (disp_var == 0) {
      Serial.print("IRIS has stopped recording. Data saved to file. Probably.");
      Serial.println();
      rec_var = 0;
      disp_var = 3;
  } else if (disp_var == 2) {
      Serial.print("IRIS is on standby.");
      Serial.println();
      disp_var = 3;
  }
  
  // Records data onto SD card/data file if rec_var is set to "on".
  if (rec_var == 1) {
    if (myICM.dataReady()) {
      myICM.getAGMT();                // The values are only updated when you call 'getAGMT'
      last_time = millis();
      printScaledAGMT(myICM.agmt);    // This function takes into account the scale settings from when the measurement was made to calculate the values with units
      delay(10);
    } else {
      Serial.println("Waiting for data");
      delay(1);
    }
  // Stops recording and closes the data file if rec_var is set to "off".
  } else if (rec_var == 0) {
    myFile.close();
    myFile = SD.open(filename);
    if (myFile) {
//      Serial.println("test.txt:");
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        Serial.write(myFile.read());
      }
      // close the file:
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
  myFile.close();
  Serial.println("Finished loop data");
  rec_var = 2;
  }
  
}

// Numerically formarts data values.
void printFormattedFloat(float val, uint8_t leading, uint8_t decimals){
  float aval = abs(val);
  if(val < 0){
    myFile.print("-");
  }else{
    myFile.print(" ");
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
      myFile.print("0");
    }else{
      break;
    }
  }
  if(val < 0){
    myFile.print(-val, decimals);
  }else{
    myFile.print(val, decimals);
  }
}

// Prints various data values to text file.
void printScaledAGMT( ICM_20948_AGMT_t agmt){
  myFile.print("Time (ms) [ ");
  myFile.print(last_time);
  myFile.print(" ], ");
  myFile.print("Scaled. Acc (mg) [ ");
  printFormattedFloat( myICM.accX(), 5, 2 );
  myFile.print(", ");
  printFormattedFloat( myICM.accY(), 5, 2 );
  myFile.print(", ");
  printFormattedFloat( myICM.accZ(), 5, 2 );
  myFile.print(" ], Gyr (DPS) [ ");
  printFormattedFloat( myICM.gyrX(), 5, 2 );
  myFile.print(", ");
  printFormattedFloat( myICM.gyrY(), 5, 2 );
  myFile.print(", ");
  printFormattedFloat( myICM.gyrZ(), 5, 2 );
  myFile.print(" ], Mag (uT) [ ");
  printFormattedFloat( myICM.magX(), 5, 2 );
  myFile.print(", ");
  printFormattedFloat( myICM.magY(), 5, 2 );
  myFile.print(", ");
  printFormattedFloat( myICM.magZ(), 5, 2 );
  myFile.print(" ], Tmp (C) [ ");
  printFormattedFloat( myICM.temp(), 5, 2 );
  myFile.print(" ]");
  myFile.println();
}
