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

// ST motion breakout IMU // Click here to get library: https://github.com/gregtomasch/USFSMAX
#include "Globals.h"
#include "Alarms.h"
#include "I2Cdev.h"
#include "USFSMAX.h"
#include "Sensor_cal.h"
#include "IMU.h"
#include "Types.h"
#include "def.h"

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
// ST motion breakout IMU
#define M_WAKE 15
#define SCL_1 16
#define SDA_1 17
#define M_INT1 24
#define M_INT2 41
#define M_INT3 25
#define M_INT4 40

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

  /* ST IMU */
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
}

// the loop routine runs over and over again forever:
void loop() {
  blink_sequence(); // Takes 4 seconds to blink LEDs
  scan(); //scan for I2C devices and output result over serial (9600 baud)
  //read_ADXL();
  //ADXLscan();

  // updates ST IMU values
  if(data_ready[0] == 1) {
    data_ready[0] = 0;
    ProcEventStatus(&i2c_0, 0);                 // I2C instance 0, Sensor instance 0 (and implicitly USFSMAX instance 0)
    FetchUSFSMAX_Data(&USFSMAX_0, &imu_0, 0);   // USFSMAX instance 0, IMU calculation instance 0 and Sensor instance 0
  }
  // prints ST data to serial monitor
  Serial.print(" ST Acc (mg) [ ");
  printFormattedFloat( 1000.0f * accData[0][0], 5, 2 );
  Serial.print(", ");
  printFormattedFloat( 1000.0f * accData[0][1], 5, 2 );
  Serial.print(", ");
  printFormattedFloat( 1000.0f * accData[0][2], 5, 2 );
  Serial.print(" ], Gyro (deg/s) [ ");
  printFormattedFloat( gyroData[0][0], 5, 2 );
  Serial.print(", ");
  printFormattedFloat( gyroData[0][1], 5, 2 );
  Serial.print(", ");
  printFormattedFloat( gyroData[0][2], 5, 2 );
  Serial.print(" ], Mag (uT) [ ");
  printFormattedFloat( magData[0][0], 5, 2 );
  Serial.print(", ");
  printFormattedFloat( magData[0][1], 5, 2 );
  Serial.print(", ");
  printFormattedFloat( magData[0][2], 5, 2 );
  Serial.print(" ], Pressure (kPa) [ ");
  printFormattedFloat( ((float)baroADC[0])*0.1f/4096.0f, 5, 2 );
  Serial.print(" ], Euler angles: yaw, pitch, roll [ ");
  printFormattedFloat( heading[0], 5, 2 );
  Serial.print(", ");
  printFormattedFloat( angle[0][1], 5, 2 );
  Serial.print(", ");
  printFormattedFloat( angle[0][0], 5, 2 );
  Serial.print(" ]");
  Serial.println();
  
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


/* Below are complicated utility functions for the ST IMU. */

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
