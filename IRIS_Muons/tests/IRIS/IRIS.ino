/* include the relevant sensor libraries */
// ST motion breakout IMU // Click here to get library: https://github.com/kriswiner/LSM6DSM_LIS2MDL_LPS22HB
#include "LIS2MDL.h" // magnetometer
#include "LPS22HB.h" // pressure and temperature sensor
#include "LSM6DSM.h" // accelerometer and gyrometer
// HSC differential pressure sensor
#include "HoneywellTruStabilitySPI.h"  // Click here to get the library: http://librarymanager/All#Honeywell_TruStability_SPI
// ADXL accelerometer
#include "ADXL372.h" // Click here to get library: https://github.com/gednrs/ADXL372
// SD card
#include <SD.h>

/* define pins */
// ST motion breakout IMU
#define M_WAKE 15
#define SCL_1 16
#define SDA_1 17
#define M_INT1 24
#define M_INT2 41
#define M_INT3 25
#define M_INT4 40
// HSC differential pressure sensor
#define CS_PITOT 33
#define SCLK_1 27
#define MISO_1 39
// ADXL accelerometer
#define HG_INT1 36
#define HG_INT2 35
#define SCLK_1 27
#define MOSI_1 26
#define MISO_1 39
#define CS_HG 38
// Muons
#define DETECTOR 14
// LED Debug
#define RGB_R 30
#define RGB_Y 31
#define RGB_G 32
// SPI flash
#define MISO_0 12
#define MOSI_0 11
#define SCLK_0 13
#define CS_MEM 10
#define RST_MEM 9
// MISC.
#define IO1_SYNC 2
#define IO2 3
#define IO3 4
#define IO4 5
#define IO5 6
#define PWR_BRD_INT 7
#define SCL_0 18
#define SDA_0 19
#define RX_MCU 21
#define TX_MCU 20

/* SPI serial communication setup */
#define SERIAL_PORT Serial
#define SPI_PORT SPI    // Your desired SPI port.       Used only when "USE_SPI" is defined
#define CS_PIN 10        // Which pin you connect CS to. Used only when "USE_SPI" is defined
#define WIRE_PORT Wire  // Your desired Wire port.      Used when "USE_SPI" is not defined
// ICM_20948_SPI myICM;  // If using SPI create an ICM_20948_SPI object

File myFile;

const int chipSelect = BUILTIN_SDCARD; // Maps to 254 I think -- make sure you select Tools > Board: Teensy 3.6 if this doesn't compile
int counter = 0;

// initializes iterative file naming variables
int filecounter = 1;
char filename[128];

// Returns the number of milliseconds passed since the Arduino board began running the current program.
unsigned int last_time;

// Variable to keep track of whether IRIS should be recording.
int rec_var = 2;
int disp_var = 2;

// Some more definitions for the BMP280 sensor.
#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 9

//Adafruit_BMP280 bme; // I2C
Adafruit_BMP280 bme(BMP_CS); // hardware SPI
//Adafruit_BMP280 bme(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

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
  
  // If the filename already exists, pick a new unique filename
  Serial.println("Checking file existence");
  while (SD.exists(filename)) {
    Serial.println("Incrementing filecounter");
    filecounter++;    // increment filecount by one
    snprintf(filename, 128, "FILE-%d.txt", filecounter);
  }
  Serial.println(filename);

  myFile = SD.open(filename, FILE_WRITE);
  
  // Display the sucess of opening the file
  if (myFile) {
    Serial.println("File successfully opened");
  } else {
    Serial.println("Error opening test.txt");
  }

  // Checks for BMP280 sensor.
    Serial.begin(9600);
  Serial.println("BMP280 test");
  
  if (!bme.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
  }
}

void loop()
{
  // Serial.available gets the number of bytes (characters) available for reading from the serial port.
  // The serial receive buffer can only store up to 64 bytes of data, so the start/stop commands are single characters.
  // To start recording, input "i" into serial monitor. To stop recording, input "o" into serial monitor.
  
  // If there is readable data inputted into the serial port, handle it:
  if (Serial.available() > 0) {
      // Serial.read() gets a character from the buffer and then throws it away, so the first one will be gone by the time you get to the elseif.
      // Saves the result of Serial.read() in a variable, then the if/elseif statements check against that variable.
      char ser_var = Serial.read();
      
      // Reads incoming serial data. Sets rec_var to "on" if 'i' is inputted into serial monitor.
      if (ser_var == 'i') {
        disp_var = 1;
        Serial.println("Now recording.");       // status message
      } else if (ser_var == 'o') { // Reads incoming serial data. Sets rec_var to "off" if 'o' is inputted into serial monitor.
        disp_var = 0;
        Serial.println("Recording stopped.");   // status message
      }
  }

  // Prints out status message to serial monitor indicating whether recording is on.
    if (disp_var == 1) {
      Serial.println("IRIS is recording.");
      rec_var = 1;
      disp_var = 3;
  } else if (disp_var == 0) {
      Serial.println("IRIS has stopped recording, data saved to file (probably).");
      rec_var = 0;
      disp_var = 3;
  } else if (disp_var == 2) {
      Serial.println("IRIS is on standby.");
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

  // Added these lines. Hopefully they can access the libraries and read pressure and altitude correctly.
  myFile.print(" ], Pressure (kPa) [ ");
  printFormattedFloat( bme.readPressure()/1000, 5, 2 );
  myFile.print(" ], Altitude (m) [ ");
  printFormattedFloat( bme.readAltitude(1013.25), 5, 2 );

  
  myFile.print(" ], Tmp (C) [ ");
  printFormattedFloat( myICM.temp(), 5, 2 );
  myFile.print(" ]");
  myFile.println();
}
