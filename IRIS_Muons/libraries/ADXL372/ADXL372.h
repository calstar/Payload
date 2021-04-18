/**/
/*********************************************************************************/
/*********************************************************************************/
/*    @author  Geraldo Daniel Neres                                              */
/*    @file    ADXL372.h                                                         */
/*    @version V1                                                                */
/*    @date    05-February-2020                                                  */
/*    @brief   SPI Implementation.                                               */
/*             This code implements the spi of the ADXL372                       */
/*             accelerometer via hardware and software.                          */
/*********************************************************************************/
/*********************************************************************************/
/**/

#ifndef ADXL372_H_
#define ADXL372_H_

#include <SPI.h>

#ifndef ARDUINO
#include <stdint.h>
#elif ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/* ADXL372 registers definition */

#define ADXL372_DEVID	                0x00u   /* Analog Devices, Inc., accelerometer ID */
#define ADXL372_DEVID_MST	            0x01u   /* Analog Devices MEMS device ID */
#define ADXL372_PARTID	            	0x02u   /* Device ID */
#define ADXL372_REVID	              	0x03u   /* product revision ID*/
#define ADXL372_STATUS_1	            0x04u   /* Status register 1 */
#define ADXL372_STATUS_2	            0x05u   /* Status register 2 */
#define ADXL372_FIFO_ENTRIES_2      	0x06u   /* Valid data samples in the FIFO */
#define ADXL372_FIFO_ENTRIES_1	      0x07u   /* Valid data samples in the FIFO */
#define ADXL372_X_DATA_H	            0x08u   /* X-axis acceleration data [11:4] */
#define ADXL372_X_DATA_L	            0x09u   /* X-axis acceleration data [3:0] */
#define ADXL372_Y_DATA_H	            0x0Au   /* Y-axis acceleration data [11:4] */
#define ADXL372_Y_DATA_L            	0x0Bu   /* Y-axis acceleration data [3:0] */
#define ADXL372_Z_DATA_H	            0x0Cu   /* Z-axis acceleration data [11:4] */
#define ADXL372_Z_DATA_L	            0x0Du   /* Z-axis acceleration data [3:0] */
#define ADXL372_X_MAXPEAK_H	          0x15u   /* X-axis MaxPeak acceleration data [15:8] */
#define ADXL372_X_MAXPEAK_L	          0x16u   /* X-axis MaxPeak acceleration data [7:0] */
#define ADXL372_Y_MAXPEAK_H	          0x17u   /* Y-axis MaxPeak acceleration data [15:8] */
#define ADXL372_Y_MAXPEAK_L         	0x18u   /* Y-axis MaxPeak acceleration data [7:0] */
#define ADXL372_Z_MAXPEAK_H	          0x19u   /* Z-axis MaxPeak acceleration data [15:8] */
#define ADXL372_Z_MAXPEAK_L	          0x1Au   /* Z-axis MaxPeak acceleration data [7:0] */
#define ADXL372_OFFSET_X	            0x20u   /* X axis offset */
#define ADXL372_OFFSET_Y	            0x21u   /* Y axis offset */
#define ADXL372_OFFSET_Z	            0x22u   /* Z axis offset */
#define ADXL372_X_THRESH_ACT_H      	0x23u   /* X axis Activity Threshold [15:8] */
#define ADXL372_X_THRESH_ACT_L      	0x24u   /* X axis Activity Threshold [7:0] */
#define ADXL372_Y_THRESH_ACT_H	      0x25u   /* Y axis Activity Threshold [15:8] */
#define ADXL372_Y_THRESH_ACT_L      	0x26u   /* Y axis Activity Threshold [7:0] */
#define ADXL372_Z_THRESH_ACT_H	      0x27u   /* Z axis Activity Threshold [15:8] */
#define ADXL372_Z_THRESH_ACT_L	      0x28u   /* Z axis Activity Threshold [7:0] */
#define ADXL372_TIME_ACT	            0x29u   /* Activity Time */
#define ADXL372_X_THRESH_INACT_H	    0x2Au   /* X axis Inactivity Threshold [15:8] */
#define ADXL372_X_THRESH_INACT_L	    0x2Bu   /* X axis Inactivity Threshold [7:0] */
#define ADXL372_Y_THRESH_INACT_H    	0x2Cu   /* Y axis Inactivity Threshold [15:8] */
#define ADXL372_Y_THRESH_INACT_L    	0x2Du   /* Y axis Inactivity Threshold [7:0] */
#define ADXL372_Z_THRESH_INACT_H	    0x2Eu   /* Z axis Inactivity Threshold [15:8] */
#define ADXL372_Z_THRESH_INACT_L	    0x2Fu   /* Z axis Inactivity Threshold [7:0] */
#define ADXL372_TIME_INACT_H         	0x30u   /* Inactivity Time [15:8] */
#define ADXL372_TIME_INACT_L        	0x31u   /* Inactivity Time [7:0] */
#define ADXL372_X_THRESH_ACT2_H     	0x32u   /* X axis Activity2 Threshold [15:8] */
#define ADXL372_X_THRESH_ACT2_L	    	0x33u   /* X axis Activity2 Threshold [7:0] */
#define ADXL372_Y_THRESH_ACT2_H     	0x34u   /* Y axis Activity2 Threshold [15:8] */
#define ADXL372_Y_THRESH_ACT2_L     	0x35u   /* Y axis Activity2 Threshold [7:0] */
#define ADXL372_Z_THRESH_ACT2_H		    0x36u   /* Z axis Activity2 Threshold [15:8] */
#define ADXL372_Z_THRESH_ACT2_L	    	0x37u   /* Z axis Activity2 Threshold [7:0] */
#define ADXL372_HPF			              0x38u   /* High Pass Filter */
#define ADXL372_FIFO_SAMPLES        	0x39u   /* FIFO Samples */
#define ADXL372_FIFO_CTL	           	0x3Au   /* FIFO Control */
#define ADXL372_INT1_MAP	            0x3Bu   /* Interrupt 1 mapping control */
#define ADXL372_INT2_MAP              0x3Cu   /* Interrupt 2 mapping control */
#define ADXL372_TIMING	             	0x3Du   /* Timing */
#define ADXL372_MEASURE	            	0x3Eu   /* Measure */
#define ADXL372_POWER_CTL             0x3Fu   /* Power control */
#define ADXL372_SELF_TEST             0x40u   /* Self Test */
#define ADXL372_RESET                 0x41u   /* Reset */
#define ADXL372_FIFO_DATA	            0x42u   /* FIFO Data */

#define ADXL372_DEVID_VAL             0xADu   /* Analog Devices, Inc., accelerometer ID */
#define ADXL372_MST_DEVID_VAL         0x1Du   /* Analog Devices MEMS device ID */
#define ADXL372_PARTID_VAL            0xFAu   /* Device ID */
#define ADXL372_REVID_VAL             0x02u   /* product revision ID*/
#define ADXL372_RESET_CODE          	0x52u	/* Writing code 0x52 resets the device */

#define ADXL372_RESERVED_BIT        	0       /* Bit reservado */

/* ADXL372_STATUS_1 */
#define ADXL372_STATUS_1_DATA_RDY(x)    (((x) >> 0) & 0x1)
#define ADXL372_STATUS_1_FIFO_RDY(x)    (((x) >> 1) & 0x1)
#define ADXL372_STATUS_1_FIFO_FULL(x)   (((x) >> 2) & 0x1)
#define ADXL372_STATUS_1_FIFO_OVR(x)    (((x) >> 3) & 0x1)
#define ADXL372_STATUS_1_USR_NVM_BUSY(x)  (((x) >> 5) & 0x1)
#define ADXL372_STATUS_1_AWAKE(x)   (((x) >> 6) & 0x1)
#define ADXL372_STATUS_1_ERR_USR_REGS(x)  (((x) >> 7) & 0x1)

/* ADXL372  MASKS */
#define MEASURE_AUTOSLEEP_MASK    0xBF
#define MEASURE_BANDWIDTH_MASK    0xF8
#define MEASURE_LOW_NOISE_MSK    0xF7
#define HPF_CORNER_MSK    0xFC
#define MEASURE_ACTPROC_MASK    0xCF
#define TIMING_ODR_MASK     0x1F
#define TIMING_WUR_MASK     0xE3
#define PWRCTRL_OPMODE_MASK   0xFC
#define PWRCTRL_INSTON_THRESH_MASK  0xDF
#define PWRCTRL_INSTON_THRESH_MASK  0xDF
#define PWRCTRL_FILTER_SETTLE_MASK  0xEF

/* ADXL372  POS */
#define MEASURE_AUTOSLEEP_POS   6
#define MEASURE_ACTPROC_POS   4
#define TIMING_ODR_POS      5
#define TIMING_WUR_POS      2
#define INSTAON_THRESH_POS    5
#define FIFO_CRL_SAMP8_POS    0
#define FIFO_CRL_MODE_POS   1
#define FIFO_CRL_FORMAT_POS   3
#define PWRCTRL_FILTER_SETTLE_POS 4
#define MEASURE_LOW_NOISE_POS    3
#define HPF_CORNER_POS 0

#define DATA_RDY  1
#define FIFO_RDY  2
#define FIFO_FULL 4
#define FIFO_OVR  8

#define ADXL_SPI_RNW    1

/*Acceleremoter configuration*/
#define ACT_VALUE          30     /* Activity threshold value */
#define INACT_VALUE        30     /* Inactivity threshold value */

typedef unsigned char adxl_spi_handle;

typedef enum {
  STAND_BY = 0,
  WAKE_UP,
  INSTANT_ON,
  FULL_BW_MEASUREMENT
} ADXL372_OP_MODE;

typedef enum {
  ODR_400Hz = 0,
  ODR_800Hz,
  ODR_1600Hz,
  ODR_3200Hz,
  ODR_6400Hz
} ADXL372_ODR;

typedef enum {
  BW_200Hz = 0,
  BW_400Hz,
  BW_800Hz,
  BW_1600Hz,
  BW_3200Hz
} ADXL372_BW;

typedef enum {
  HPF_CORNER0 = 0,
  HPF_CORNER1,
  HPF_CORNER2,
  HPF_CORNER3
} ADXL372_HPF_CORNER;

typedef enum {
  BYPASSED = 0,
  STREAMED,
  TRIGGERED,
  OLDEST_SAVED
} ADXL372_FIFO_MODE;

typedef enum {
  FILTER_SETTLE_370 = 0,
  FILTER_SETTLE_16    
} ADXL372_Filter_Settle;


typedef struct {
  short x;
  short y;
  short z;
} AccelTriplet_t;

struct ADXL372_AccelTriplet {
  int16_t x;
  int16_t y;
  int16_t z;
};

struct ADXL372_AccelTripletG {
  float x;
  float y;
  float z;
};

struct ADXL372_AccelTripletMs2 {
  float x;
  float y;
  float z;
};

struct ADXL372_Register {
  uint8_t b7;
  uint8_t b6;
  uint8_t b5;
  uint8_t b4;
  uint8_t b3;
  uint8_t b2;
  uint8_t b1;
  uint8_t b0;
};

class ADXL372 {
  public:
    ADXL372(int8_t cspin, SPIClass *theSPI = &SPI);
    ADXL372(int8_t cspin, int8_t mosipin, int8_t misopin, int8_t sckpin);

    uint8_t begin();
    
    uint8_t _cs;
    uint8_t getDeviceID();
    uint8_t GetDeviceID();
    uint8_t GetPartID();
    uint8_t GetRevID();
    uint8_t GetDevIDMst();
    uint8_t AddressToSPIData(uint8_t address, uint8_t rw_mask);
    uint8_t ReadAccTriplet(struct ADXL372_AccelTriplet *triplet);

    int16_t ConvertFrom2Complement(uint8_t *high_part, uint8_t *low_part);

    struct ADXL372_AccelTripletG ConvertAccTripletToG(const struct ADXL372_AccelTriplet *triplet) ;
    struct ADXL372_AccelTripletMs2 ConvertAccTripletToMs2(const struct ADXL372_AccelTriplet *triplet);

    void defaultConfigurations();
    void registryData();
    void reset();
    void SetAxisOffsets();
    void GetStatus(uint8_t *status1, uint8_t *status2, uint16_t *fifo_entries);
    uint8_t Update_reg(uint8_t reg_addr, uint8_t mask, uint8_t shift, uint8_t data);

    int Set_op_mode(ADXL372_OP_MODE mode);
    int Set_ODR(ADXL372_ODR odr);
    int Set_BandWidth(ADXL372_BW bw);
    int Set_low_noise(bool noise);
    int Set_hpf_corner(ADXL372_HPF_CORNER corner);

  protected:
    uint8_t spiTransfer(uint8_t x = 0xFF);
    uint8_t readRegister(uint8_t reg);
    uint8_t readRegisterM(uint8_t reg);

    void writeRegister(uint8_t reg, uint8_t value);


  private:

    SPIClass *SPIinterface;

    int8_t _mosi, _miso, _sck;

};
#endif
