/* include the relevant sensor libraries */
// ADXL accelerometer
#include "ADXL372.h" // Click here to get library: https://github.com/gednrs/ADXL372
// ST motion breakout IMU // Click here to get library: https://github.com/gregtomasch/USFSMAX
// missing libraries:
// <i2c_t3.h> (only compatible with Teensy 3.x, replace with https://www.pjrc.com/teensy/td_libs_Wire.html)
// "Global.h" (required unknown <FS.h> library)
#include "Alarms.h"
#include "I2Cdev.h"
#include "USFSMAX.h"
#include "Sensor_cal.h"
#include "IMU.h"
#include "Types.h"
#include "def.h"
//// incorrect library: https://github.com/kriswiner/LSM6DSM_LIS2MDL_LPS22HB
//#include "LIS2MDL.h" // magnetometer
//#include "LPS22HB.h" // pressure and temperature sensor
//#include "LSM6DSM.h" // accelerometer and gyrometer
// HSC differential pressure sensor (as of 04/11/2021: not currently on board)
#include "HoneywellTruStabilitySPI.h"  // Click here to get the library: http://librarymanager/All#Honeywell_TruStability_SPI
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
#define SPI_PORT SPI      // your desired SPI port          used only when "USE_SPI" is defined
#define CS_PIN 10         // which pin you connect CS to    used only when "USE_SPI" is defined
#define WIRE_PORT Wire    // your desired Wire port         used when "USE_SPI" is not defined

/* SPI objects (via hardware) */
ADXL372 myADXL = ADXL372(CS_HG, MOSI_1, MISO_1, SCLK_1);

/* class objects and functions from sensors */
// ADXL
struct ADXL372_AccelTriplet accel;
struct ADXL372_AccelTripletG accelG;
// ST IMU
I2Cdev     i2c_0(&SENSOR_0_WIRE_INSTANCE);
USFSMAX    USFSMAX_0(&i2c_0, 0);
IMU        imu_0(&USFSMAX_0, 0);
Sensor_cal sensor_cal(&i2c_0, &USFSMAX_0, 0);
void       ProcEventStatus(I2Cdev* i2c_BUS, uint8_t sensorNUM);
void       FetchUSFSMAX_Data(USFSMAX* usfsmax, IMU* IMu, uint8_t sensorNUM);
void       DRDY_handler_0();
void       SerialInterface_handler();

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

  /* ST IMU */
  /**
  // Set up DRDY (magnetometer) interrupt pin
  pinMode(M_INT1, INPUT);
  // Initialize USFSMAX_0 I2C bus
  SENSOR_0_WIRE_INSTANCE.begin(I2C_MASTER, 0x00, SDA_1, I2C_PULLUP_EXT, SCL_1);
  delay(100);
  SENSOR_0_WIRE_INSTANCE.setClock(I2C_RATE_100);  // Set I2C clock speed to 100kHz cor configuration
  delay(2000);
  // Do I2C bus scan if serial debug is active
  #ifdef SERIAL_DEBUG                             // Should see MAX32660 slave bus address (default is 0x57)
    i2c_0.I2Cscan();                                           
  #endif
  // Initialize USFSMAX_0
  #ifdef SERIAL_DEBUG
    Serial.print("Initializing USFSMAX_0...");
    Serial.println("");
  #endif
  USFSMAX_0.init_USFSMAX();                       // Configure USFSMAX and sensors 
  SENSOR_0_WIRE_INSTANCE.setClock(I2C_CLOCK);     // Set the I2C clock to high speed for run-mode data collection
  delay(100);
  **/

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

    /**
    // updates ST IMU values
    if(data_ready[0] == 1) {
      data_ready[0] = 0;
      ProcEventStatus(&i2c_0, 0);                 // I2C instance 0, Sensor instance 0 (and implicitly USFSMAX instance 0)
      FetchUSFSMAX_Data(&USFSMAX_0, &imu_0, 0);   // USFSMAX instance 0, IMU calculation instance 0 and Sensor instance 0
    }
    **/

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

  /**
  // ST IMU sensor and raw quaternion outout
  myFile.print(" ST Acc (mg) [ ");
  printFormattedFloat( 1000.0f * accData[0][0], 5, 2 );
  myFile.print(", ");
  printFormattedFloat( 1000.0f * accData[0][1], 5, 2 );
  myFile.print(", ");
  printFormattedFloat( 1000.0f * accData[0][2], 5, 2 );
  myFile.print(" ], Gyro (deg/s) [ ");
  printFormattedFloat( gyroData[0][0], 5, 2 );
  myFile.print(", ");
  printFormattedFloat( gyroData[0][1], 5, 2 );
  myFile.print(", ");
  printFormattedFloat( gyroData[0][2], 5, 2 );
  myFile.print(" ], Mag (uT) [ ");
  printFormattedFloat( magData[0][0], 5, 2 );
  myFile.print(", ");
  printFormattedFloat( magData[0][1], 5, 2 );
  myFile.print(", ");
  printFormattedFloat( magData[0][2], 5, 2 );
  myFile.print(" ], Pressure (kPa) [ ");
  printFormattedFloat( ((float)baroADC[0])*0.1f/4096.0f, 5, 2 );
  myFile.print(" ], Euler angles: yaw, pitch, roll [ ");
    printFormattedFloat( heading[0], 5, 2 );
  myFile.print(", ");
  printFormattedFloat( angle[0][1], 5, 2 );
  myFile.print(", ");
  printFormattedFloat( angle[0][0], 5, 2 );
  myFile.print(" ]");
  myFile.println();
  **/
}

/* Below are complicated utility functions for the ST IMU you don't need to understand. */
/**
void ProcEventStatus(I2Cdev* i2c_BUS, uint8_t sensorNUM)
{
  uint8_t temp[1];

  // Read algorithm status and event status
  i2c_BUS->readBytes(MAX32660_SLV_ADDR, COMBO_DRDY_STAT, 1, temp);
  eventStatus[sensorNUM] = temp[0];

  // Decode the event status to determine what data is ready and set the appropriate DRDY fags
  if(eventStatus[sensorNUM] & 0x01) Gyro_flag[sensorNUM] = 1;
  if(eventStatus[sensorNUM] & 0x02) Acc_flag[sensorNUM]  = 1;
  if(eventStatus[sensorNUM] & 0x04) Mag_flag[sensorNUM]  = 1;
  if(eventStatus[sensorNUM] & 0x08) Baro_flag[sensorNUM] = 1;
  if(eventStatus[sensorNUM] & 0x10) Quat_flag[sensorNUM] = 1;
}

void FetchUSFSMAX_Data(USFSMAX* usfsmax, IMU* IMu, uint8_t sensorNUM)
{
  uint8_t call_sensors = eventStatus[sensorNUM] & 0x0F;

  Acq_time = 0;
  Begin = micros();

  // Optimize the I2C read function with respect to whatever sensor data is ready
  switch(call_sensors)
  {
   case 0x01:
     usfsmax->GyroAccel_getADC();
     break;
   case 0x02:
     usfsmax->GyroAccel_getADC();
     break;
   case 0x03:
     usfsmax->GyroAccel_getADC();
     break;
   case 0x07:
     usfsmax->GyroAccelMagBaro_getADC();
     break;
   case 0x0B:
     usfsmax->GyroAccelMagBaro_getADC();
     break;
   case 0x0F:
     usfsmax->GyroAccelMagBaro_getADC();
     break;
   case 0x0C:
     usfsmax->MagBaro_getADC();
     break;
   case 0x04:
     usfsmax->MAG_getADC();
     break;
   case 0x08:
     usfsmax->BARO_getADC();
     break;
   default:
     break;
  };
  Acq_time += micros() - Begin;

  if(Mag_flag[sensorNUM])
  {
    if(ScaledSensorDataFlag)                                                                                         // Calibration data is applied in the coprocessor; just scale
    {
      for(uint8_t i=0; i<3; i++)
      {
        magData[sensorNUM][i] = ((float)magADC[sensorNUM][i])*UT_per_Count;
      }
    } else                                                                                                           // Calibration data applied locally
    {
      sensor_cal.apply_adv_calibration(ellipsoid_magcal[sensorNUM], magADC[sensorNUM], UT_per_Count, mag_calData[sensorNUM]);
      sensor_cal.apply_adv_calibration(final_magcal[sensorNUM], mag_calData[sensorNUM], 1.0f, sensor_point);
      MAG_ORIENTATION(sensor_point[0], sensor_point[1], sensor_point[2]);
    }
    Mag_flag[sensorNUM] = 0;
  }
  if(Acc_flag[sensorNUM])
  {
    if(ScaledSensorDataFlag)                                                                                         // Calibration data is applied in the coprocessor; just scale
    {
      for(uint8_t i=0; i<3; i++)
      {
        accData[sensorNUM][i] = ((float)accADC[sensorNUM][i])*g_per_count;
      } 
    } else                                                                                                           // Calibration data applied locally
    {
      sensor_cal.apply_adv_calibration(accelcal[sensorNUM], accADC[sensorNUM], g_per_count, sensor_point);
      ACC_ORIENTATION(sensor_point[0], sensor_point[1], sensor_point[2]);
    }
    Acc_flag[sensorNUM] = 0;
  }
  if(Gyro_flag[sensorNUM] == 1)
  {
    if(ScaledSensorDataFlag)                                                                                         // Calibration data is applied in the coprocessor; just scale
    {
      for(uint8_t i=0; i<3; i++)
      {
        gyroData[sensorNUM][i] = ((float)gyroADC[sensorNUM][i])*dps_per_count;
      }
    } else                                                                                                           // Calibration data applied locally
    {
      sensor_cal.apply_adv_calibration(gyrocal[sensorNUM], gyroADC[sensorNUM], dps_per_count, sensor_point);
      GYRO_ORIENTATION(sensor_point[0], sensor_point[1], sensor_point[2]);
    }

    // Call alternative (Madgwick or Mahony) IMU fusion filter
    IMu->compute_Alternate_IMU();
    Gyro_flag[sensorNUM] = 0;
  }
  if(Quat_flag[sensorNUM] == 1)
  {
    IMu->computeIMU();
    Quat_flag[sensorNUM] = 0;
  }
}

// Host DRDY interrupt handler
void DRDY_handler_0()
{
  data_ready[0] = 1;
}

// Serial interface handler
void SerialInterface_handler()
{
  serial_input = 0;
  if(Serial.available()) serial_input = Serial.read();
  if(serial_input == 49) {sensor_cal.GyroCal();}                                                                     // Type "1" to initiate USFSMAX_0 Gyro Cal
  if(serial_input == 50)                                                                                             // Type "2" to list current sensor calibration data
  {
    SENSOR_0_WIRE_INSTANCE.setClock(I2C_RATE_100);                                                                   // Set I2C clock to 100kHz to read the calibration data from the MAX32660
    delay(100);
    USFSMAX_0.Retreive_full_gyrocal();
    delay(100);
    USFSMAX_0.Retreive_full_accelcal();
    delay(100);
    USFSMAX_0.Retreive_ellip_magcal();
    delay(100);
    USFSMAX_0.Retreive_final_magcal();
    delay(100);
    SENSOR_0_WIRE_INSTANCE.setClock(I2C_CLOCK);                                                                      // Resume high-speed I2C operation
    delay(100);

    // Print the calibration results
    Serial.println("Gyroscope Sensor Offsets (dps)");
    Serial.println(gyrocal[0].V[0], 4);
    Serial.println(gyrocal[0].V[1], 4);
    Serial.println(gyrocal[0].V[2], 4); Serial.println("");
    Serial.println("Gyroscope Calibration Tensor");
    Serial.print(gyrocal[0].invW[0][0], 4); Serial.print(",");
    Serial.print(gyrocal[0].invW[0][1], 4); Serial.print(",");
    Serial.println(gyrocal[0].invW[0][2], 4);
    Serial.print(gyrocal[0].invW[1][0], 4); Serial.print(",");
    Serial.print(gyrocal[0].invW[1][1], 4); Serial.print(",");
    Serial.println(gyrocal[0].invW[1][2], 4);
    Serial.print(gyrocal[0].invW[2][0], 4); Serial.print(",");
    Serial.print(gyrocal[0].invW[2][1], 4); Serial.print(",");
    Serial.println(gyrocal[0].invW[2][2], 4);
    Serial.println(""); Serial.println("");
    Serial.println("Accelerometer Sensor Offsets (g)");
    Serial.println(accelcal[0].V[0], 4);
    Serial.println(accelcal[0].V[1], 4);
    Serial.println(accelcal[0].V[2], 4); Serial.println("");
    Serial.println("Accelerometer Calibration Tensor");
    Serial.print(accelcal[0].invW[0][0], 4); Serial.print(",");
    Serial.print(accelcal[0].invW[0][1], 4); Serial.print(",");
    Serial.println(accelcal[0].invW[0][2], 4);
    Serial.print(accelcal[0].invW[1][0], 4); Serial.print(",");
    Serial.print(accelcal[0].invW[1][1], 4); Serial.print(",");
    Serial.println(accelcal[0].invW[1][2], 4);
    Serial.print(accelcal[0].invW[2][0], 4); Serial.print(",");
    Serial.print(accelcal[0].invW[2][1], 4); Serial.print(",");
    Serial.println(accelcal[0].invW[2][2], 4);
    Serial.println(""); Serial.println("");
    Serial.println("Magnetometer Sensor Offsets (uT)");
    Serial.println(ellipsoid_magcal[0].V[0], 4);
    Serial.println(ellipsoid_magcal[0].V[1], 4);
    Serial.println(ellipsoid_magcal[0].V[2], 4); 
    Serial.println("");
    Serial.println("Magnetometer Soft Iron Correction Tensor");
    Serial.print(ellipsoid_magcal[0].invW[0][0], 4); Serial.print(",");
    Serial.print(ellipsoid_magcal[0].invW[0][1], 4); Serial.print(",");
    Serial.println(ellipsoid_magcal[0].invW[0][2], 4);
    Serial.print(ellipsoid_magcal[0].invW[1][0], 4); Serial.print(",");
    Serial.print(ellipsoid_magcal[0].invW[1][1], 4); Serial.print(",");
    Serial.println(ellipsoid_magcal[0].invW[1][2], 4);
    Serial.print(ellipsoid_magcal[0].invW[2][0], 4); Serial.print(",");
    Serial.print(ellipsoid_magcal[0].invW[2][1], 4); Serial.print(",");
    Serial.println(ellipsoid_magcal[0].invW[2][2], 4);
    Serial.println(""); Serial.println("");
    Serial.println("Magnetometer Residual Hard Iron Offsets (uT)");
    Serial.println(final_magcal[0].V[0], 4);
    Serial.println(final_magcal[0].V[1], 4);
    Serial.println(final_magcal[0].V[2], 4);
    Serial.println("");
    Serial.println("Magnetometer Fine Calibration/Alignment Tensor");
    Serial.print(final_magcal[0].invW[0][0], 4); Serial.print(",");
    Serial.print(final_magcal[0].invW[0][1], 4); Serial.print(",");
    Serial.println(final_magcal[0].invW[0][2], 4);
    Serial.print(final_magcal[0].invW[1][0], 4); Serial.print(",");
    Serial.print(final_magcal[0].invW[1][1], 4); Serial.print(",");
    Serial.println(final_magcal[0].invW[1][2], 4);
    Serial.print(final_magcal[0].invW[2][0], 4); Serial.print(",");
    Serial.print(final_magcal[0].invW[2][1], 4); Serial.print(",");
    Serial.println(final_magcal[0].invW[2][2], 4);
    Serial.println(""); Serial.println("");
    sensor_cal.sendOneToProceed();                                                                                   // Halt the serial monitor to let the user read the calibration data
  }
  if(serial_input == 51) {USFSMAX_0.Reset_DHI();}                                                                    // Type "3" to reset the DHI corrector
  serial_input = 0;
  
  // Hotkey messaging
  Serial.println("'1' Gyro Cal");
  Serial.println("'2' List Cal Data");
  Serial.println("'3' Reset DHI Corrector");
  Serial.println("");
}
**/
