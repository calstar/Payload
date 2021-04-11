// Demonstração básica para leituras do acelerômetro ADXL372

#include <SPI.h>
#include "ADXL372.h"

/* Usado para SPI via software */
#define ADXL372_CLK  18
#define ADXL372_MISO 19
#define ADXL372_MOSI 23
/* Usado para SPI via software e hardware */
#define ADXL372_CS   5

/* SPI via software */
/* ADXL372 adxl = ADXL372(ADXL372_CS, ADXL372_MOSI, ADXL372_MISO, ADXL372_CLK); */
/* SPI via hardware */
ADXL372 adxl = ADXL372(ADXL372_CS);

struct ADXL372_AccelTriplet accel;
struct ADXL372_AccelTripletG accelG;

void setup(void) {

  Serial.begin(115200);
  
  if(adxl.begin() == ADXL372_DEVID_VAL)
    Serial.println("ADXL372 found!");
  else
    Serial.println("ADXL372 don't found!");
  
  /* Seta a banda do acelerômetro */
  adxl.Set_BandWidth(BW_3200Hz);
  /* Seta o modo de baixo ruído */
  adxl.Set_low_noise(true);
  /* Seta o ODR */
  adxl.Set_ODR(ODR_6400Hz);
  /* Seta a frequência de corte do HPF */
  adxl.Set_hpf_corner(HPF_CORNER0);
  /* Seta o modo de operação */
  adxl.Set_op_mode(FULL_BW_MEASUREMENT);
  
  delay(100);

}

void loop() {

  adxl.ReadAccTriplet(&accel);
  accelG = adxl.ConvertAccTripletToG(&accel);

  Serial.print(accelG.x);
  Serial.print("\t " );
  Serial.print(accelG.y);                  
  Serial.print("\t " );
  Serial.println(accelG.z);
}
