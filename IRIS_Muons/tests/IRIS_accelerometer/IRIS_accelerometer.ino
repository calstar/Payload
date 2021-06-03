/* include the relevant sensor libraries */
// ADXL accelerometer
#include "ADXL372.h" // Click here to get library: https://github.com/gednrs/ADXL372
// SD card libraries:
#include <SPI.h>
#include <SD.h>

/* define pins */
// ADXL accelerometer
#define HG_INT1 36
#define HG_INT2 35
#define SCLK_1 27
#define MOSI_1 26
#define MISO_1 39
#define CS_HG 38
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
// Muons
#define DETECTOR 14
#define IO1_SYNC 2
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
#define SPI_PORT SPI      // your desired SPI port          used only when "USE_SPI" is defined
#define CS_PIN 10         // which pin you connect CS to    used only when "USE_SPI" is defined
#define WIRE_PORT Wire    // your desired Wire port         used when "USE_SPI" is not defined

/* SPI objects (via hardware) */
ADXL372 myADXL = ADXL372(CS_HG, MOSI_1, MISO_1, SCLK_1);

/* class objects and functions from sensors */
// ADXL
struct ADXL372_AccelTriplet accel;
struct ADXL372_AccelTripletG accelG;

// Teensy 3.5 & 3.6 & 4.1 on-board: BUILTIN_SDCARD
const int chipSelect = BUILTIN_SDCARD; // Maps to 254 I think -- make sure you select Tools > Board: Teensy 4.1 if this doesn't compile
int counter = 0;

// initializes file and iterative file naming variables
File myFile;
int filecounter = 1;
char filename[128];

// Returns the number of milliseconds passed since the Teensy board began running the current program.
unsigned int last_time;

// variable to keep track of whether IRIS/Muons should be recording
int rec_var = 2;

void setup() { 
  last_time = millis();
  snprintf(filename, 128, "FILE-%d.txt", filecounter);

  /* Serial */
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial.println("Opened serial connection.");
  /**
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  **/

  /* SD card */
  // Start SPI
  SPI_PORT.begin();
  //Attempt to initialize the SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) { // Uses SDHC not SPI
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  /* ADXL */
  // attempt to initialize ADXL
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

  /* File */
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

  /* Recording Instruction Message */
  Serial.println("To start recording, type ON into serial monitor. To stop recording, type OFF into serial monitor.");
}

void loop() {
  // if there is readable data inputted into the serial port:
  if (Serial.available() > 0) {
    // reads incoming serial data as string
    String command = Serial.readStringUntil('\n');
    char temp = Serial.read(); // remove invisible characters left in buffer
    
    if (command == "ON") {
      rec_var = 1;
      Serial.println("IRIS is recording.");
    } else if (command == "OFF") {
      rec_var = 0;
      Serial.println("IRIS has stopped recording, data saved to file (probably).");
    }
  }
  
  // Records data onto SD card/data file if rec_var is set to "on".
  if (rec_var == 1) {
    last_time = millis();
    
    // the ADXL values are only updated when you call 'ReadAccTriplet'
    myADXL.ReadAccTriplet(&accel);
    accelG = myADXL.ConvertAccTripletToG(&accel);

    // This function takes into account the scale settings from when the measurement was made to calculate the values with units
    printScaledData();
    delay(10);
  // Stops recording and closes the data file if rec_var is set to "off".
  } else if (rec_var == 0) {
    myFile.close();
    Serial.println("Finished loop data");
    rec_var = 2;
  }
}

// Numerically formats data values.
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

// Prints various scaled data values to text file.
void printScaledData(){
  // prints time
  myFile.print("Time (ms) [ ");
  myFile.print(last_time);
  myFile.print(" ], ");
  
  // ADXL values
  myFile.print("ADXL Acc (mg) [ ");
  printFormattedFloat( accelG.x / 1000.0, 5, 2 );
  myFile.print(", ");
  printFormattedFloat( accelG.y / 1000.0, 5, 2 );
  myFile.print(", ");
  printFormattedFloat( accelG.z / 1000.0, 5, 2 );
  myFile.print(" ]");
  myFile.println();
}
