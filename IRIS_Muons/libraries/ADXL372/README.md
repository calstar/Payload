# ADXL372 Accelerometer Driver for ESP32
Arduino Library for ADXL372. This code implements the SPI of the ADXL372 accelerometer via hardware and software.

## About the ADXL372 ##
The ADXL372 is a digital accelerometer that supports both SPI and I2C mode and 'range' of 200g.

More information on the ADXL372 can be found in the datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/ADXL372.pdf

## Compatibility ##

MCU                | Tested Works | Doesn't Work | Not Tested  | Notes
------------------ | :----------: | :----------: | :---------: | -----
ESP32              |      X       |             |            | Supports 10MHz SPI
ESP8266            |      X       |             |            | Supports 10MHz SPI

