/*
ICM20948.h
Rajiv Govindjee
rgovindjee@berkeley.edu

Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
and associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, 
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or 
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ICM20948_h
#define ICM20948_h

#include "Arduino.h"
#include "SPI.h"     // SPI library

class ICM20948{
  public:
    enum GyroRange
    {
      GYRO_RANGE_250DPS,
      GYRO_RANGE_500DPS,
      GYRO_RANGE_1000DPS,
      GYRO_RANGE_2000DPS
    };
    enum AccelRange
    {
      ACCEL_RANGE_2G,
      ACCEL_RANGE_4G,
      ACCEL_RANGE_8G,
      ACCEL_RANGE_16G    
    };
    enum DlpfBandwidth
    {
      DLPF_BANDWIDTH_184HZ,
      DLPF_BANDWIDTH_92HZ,
      DLPF_BANDWIDTH_41HZ,
      DLPF_BANDWIDTH_20HZ,
      DLPF_BANDWIDTH_10HZ,
      DLPF_BANDWIDTH_5HZ
    };
    enum LpAccelOdr
    {
      LP_ACCEL_ODR_0_24HZ = 0,
      LP_ACCEL_ODR_0_49HZ = 1,
      LP_ACCEL_ODR_0_98HZ = 2,
      LP_ACCEL_ODR_1_95HZ = 3,
      LP_ACCEL_ODR_3_91HZ = 4,
      LP_ACCEL_ODR_7_81HZ = 5,
      LP_ACCEL_ODR_15_63HZ = 6,
      LP_ACCEL_ODR_31_25HZ = 7,
      LP_ACCEL_ODR_62_50HZ = 8,
      LP_ACCEL_ODR_125HZ = 9,
      LP_ACCEL_ODR_250HZ = 10,
      LP_ACCEL_ODR_500HZ = 11
    };

    ICM20948(SPIClass &bus,uint8_t csPin);
    int begin();
    int setAccelRange(AccelRange range);
    int setGyroRange(GyroRange range);
    int setDlpfBandwidth(DlpfBandwidth bandwidth);
    int setSrd(uint8_t srd);
    int enableDataReadyInterrupt();
    int disableDataReadyInterrupt();
    int enableWakeOnMotion(float womThresh_mg,LpAccelOdr odr);
    int readSensor();
    float getAccelX_mss();
    float getAccelY_mss();
    float getAccelZ_mss();
    float getGyroX_rads();
    float getGyroY_rads();
    float getGyroZ_rads();
    float getMagX_uT();
    float getMagY_uT();
    float getMagZ_uT();
    float getTemperature_C();
    
    int calibrateGyro();
    float getGyroBiasX_rads();
    float getGyroBiasY_rads();
    float getGyroBiasZ_rads();
    void setGyroBiasX_rads(float bias);
    void setGyroBiasY_rads(float bias);
    void setGyroBiasZ_rads(float bias);
    int calibrateAccel();
    float getAccelBiasX_mss();
    float getAccelScaleFactorX();
    float getAccelBiasY_mss();
    float getAccelScaleFactorY();
    float getAccelBiasZ_mss();
    float getAccelScaleFactorZ();
    void setAccelCalX(float bias,float scaleFactor);
    void setAccelCalY(float bias,float scaleFactor);
    void setAccelCalZ(float bias,float scaleFactor);
    int calibrateMag();
    float getMagBiasX_uT();
    float getMagScaleFactorX();
    float getMagBiasY_uT();
    float getMagScaleFactorY();
    float getMagBiasZ_uT();
    float getMagScaleFactorZ();
    void setMagCalX(float bias,float scaleFactor);
    void setMagCalY(float bias,float scaleFactor);
    void setMagCalZ(float bias,float scaleFactor);

    const uint8_t WHO_AM_I = 0x00;
    //USER BANK 0
    
    const uint8_t ACCEL_OUT = 0x2D;
    const uint8_t GYRO_OUT = 0x33;
    const uint8_t TEMP_OUT = 0x39;
    const uint8_t EXT_SENS_DATA_00 = 0x38;
    const uint8_t DATA_RDY_STATUS = 0x74;
    const uint8_t PWR_MGMNT_1 = 0x06;
    const uint8_t PWR_MGMNT_2 = 0x07;
    const uint8_t PWR_RESET = 0x06;
    const uint8_t INT_PIN_CFG = 0x0F;
    const uint8_t INT_ENABLE = 0x10;
    const uint8_t FIFO_EN = 0x66;
    const uint8_t FIFO_COUNT = 0x70;
    const uint8_t FIFO_READ = 0x72;

    //Not sure about these, left over from the MPU9250
    const uint8_t FIFO_TEMP = 0x80;
    const uint8_t FIFO_GYRO = 0x70;
    const uint8_t FIFO_ACCEL = 0x08;
    const uint8_t FIFO_MAG = 0x01;

    //USER BANK 1, think these are done by them?
    const uint8_t XA_OFFS = 0x14;
    const uint8_t YA_OFFS = 0x17;
    const uint8_t ZA_OFFS = 0x1A;

    //USER BANK 2
    const uint8_t ACCEL_SMPLRT_DIV = 0x16;
    const uint8_t ACCEL_CONFIG = 0x14;
     const uint8_t ACCEL_CONFIG2 = 0x15;
    //No Digital Low-Pass Filter
    const uint8_t ACCEL_FS_SEL_2G = 0x00;
    const uint8_t ACCEL_FS_SEL_4G = 0x02;
    const uint8_t ACCEL_FS_SEL_8G = 0x05;
    const uint8_t ACCEL_FS_SEL_16G = 0x06;

    const uint8_t GYRO_SMPLRT_DIV = 0x00;
    const uint8_t GYRO_CONFIG_1 = 0x01;
    const uint8_t GYRO_CONFIG_2 = 0x02;
    const uint8_t GYRO_FS_SEL_250DPS = 0x00;
    const uint8_t GYRO_FS_SEL_500DPS = 0x02;
    const uint8_t GYRO_FS_SEL_1000DPS = 0x05;
    const uint8_t GYRO_FS_SEL_2000DPS = 0x06;
    const uint8_t XG_OFFS_USR = 0x03;
    const uint8_t YG_OFFS_USR = 0x05;
    const uint8_t ZG_OFFS_USR = 0x07;

    const uint8_t TEMP_CONFIG = 0x53;

    const uint8_t USER_CTRL = 0x6A;

    const uint8_t REG_BANK_SEL = 0x7F; //this is the same for all banks; select bank first, then address when reading

  protected:
    // spi
    SPIClass *_spi;
    uint8_t _csPin;
    bool _useSPI;
    bool _useSPIHS;
    const uint8_t SPI_READ = 0x80;
    const uint32_t SPI_LS_CLOCK = 1000000;  // 1 MHz
    const uint32_t SPI_HS_CLOCK = 7000000; // 15 MHz
    // track success of interacting with sensor
    int _status;
    // buffer for reading from sensor
    uint8_t _buffer[21];
    // data counts
    int16_t _axcounts,_aycounts,_azcounts;
    int16_t _gxcounts,_gycounts,_gzcounts;
    int16_t _hxcounts,_hycounts,_hzcounts;
    int16_t _tcounts;
    // data buffer
    float _ax, _ay, _az;
    float _gx, _gy, _gz;
    float _hx, _hy, _hz;
    float _t;
    // wake on motion
    uint8_t _womThreshold;
    // scale factors
    float _accelScale;
    float _gyroScale;
    float _magScaleX, _magScaleY, _magScaleZ;
    const float _tempScale = 333.87f;
    const float _tempOffset = 21.0f;
    // configuration
    AccelRange _accelRange;
    GyroRange _gyroRange;
    DlpfBandwidth _bandwidth;
    uint8_t _srd;
    // gyro bias estimation
    size_t _numSamples = 100;
    double _gxbD, _gybD, _gzbD;
    float _gxb, _gyb, _gzb;
    // accel bias and scale factor estimation
    double _axbD, _aybD, _azbD;
    float _axmax, _aymax, _azmax;
    float _axmin, _aymin, _azmin;
    float _axb, _ayb, _azb;
    float _axs = 1.0f;
    float _ays = 1.0f;
    float _azs = 1.0f;
    // magnetometer bias and scale factor estimation
    uint16_t _maxCounts = 1000;
    float _deltaThresh = 0.3f;
    uint8_t _coeff = 8;
    uint16_t _counter;
    float _framedelta, _delta;
    float _hxfilt, _hyfilt, _hzfilt;
    float _hxmax, _hymax, _hzmax;
    float _hxmin, _hymin, _hzmin;
    float _hxb, _hyb, _hzb;
    float _hxs = 1.0f;
    float _hys = 1.0f;
    float _hzs = 1.0f;
    float _avgs;
    // transformation matrix
    /* transform the accel and gyro axes to match the magnetometer axes */
    const int16_t tX[3] = {0,  1,  0}; 
    const int16_t tY[3] = {1,  0,  0};
    const int16_t tZ[3] = {0,  0, -1};
    // constants
    const float G = 9.807f;
    const float _d2r = 3.14159265359f/180.0f;
    // ICM20948 registers

    

    // private functions
    int writeRegister(uint8_t subAddress, uint8_t data);
    int readRegisters(uint8_t subAddress, uint8_t count, uint8_t* dest);
    int whoAmI();
};

class ICM20948FIFO: public ICM20948 {
  public:
    using ICM20948::ICM20948;
    int enableFifo(bool accel,bool gyro,bool mag,bool temp);
    int readFifo();
    void getFifoAccelX_mss(size_t *size,float* data);
    void getFifoAccelY_mss(size_t *size,float* data);
    void getFifoAccelZ_mss(size_t *size,float* data);
    void getFifoGyroX_rads(size_t *size,float* data);
    void getFifoGyroY_rads(size_t *size,float* data);
    void getFifoGyroZ_rads(size_t *size,float* data);
    void getFifoMagX_uT(size_t *size,float* data);
    void getFifoMagY_uT(size_t *size,float* data);
    void getFifoMagZ_uT(size_t *size,float* data);
    void getFifoTemperature_C(size_t *size,float* data);
  protected:
    // fifo
    bool _enFifoAccel,_enFifoGyro,_enFifoMag,_enFifoTemp;
    size_t _fifoSize,_fifoFrameSize;
    float _axFifo[85], _ayFifo[85], _azFifo[85];
    size_t _aSize;
    float _gxFifo[85], _gyFifo[85], _gzFifo[85];
    size_t _gSize;
    float _hxFifo[73], _hyFifo[73], _hzFifo[73];
    size_t _hSize;
    float _tFifo[256];
    size_t _tSize;
};

#endif
