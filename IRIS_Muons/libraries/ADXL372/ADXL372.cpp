/**/
/*********************************************************************************/
/*********************************************************************************/
/*    @author  Geraldo Daniel Neres                                              */
/*    @file    ADXL372.cpp                                                       */
/*    @version v1                                                                */
/*    @date    05-February-2020                                                  */
/*    @brief   SPI Implementation.                                               */
/*             This code implements the spi of the ADXL372                       */
/*             accelerometer via hardware and software.                          */
/*********************************************************************************/
/*********************************************************************************/
/**/

#include "ADXL372.h"
#include "Arduino.h"

/*!
     @brief  Instancia uma nova classe ADXL372 usando SPI via hardware
     @param  cspin
             Pino do Chip Select (5)
     @param  *theSPI
             Parâmetro opcional, contém o objeto SPI
*/
ADXL372::ADXL372(int8_t cspin, SPIClass *theSPI) {
  _cs = cspin;
  _mosi = -1;
  _miso = -1;
  _sck = -1;

  SPIinterface = theSPI;
}

/*!
     @brief  Instancia uma nova classe ADXL372 usando SPI via software
     @param  cspin
             Pino do Chip Select (5)
     @param  mosipin
             Pino do MOSI (23)
     @param  misopin
             Pino do MISO (19)
     @param  sckpin
             Pino do CLK (18)
*/
ADXL372::ADXL372(int8_t cspin, int8_t mosipin, int8_t misopin,
                 int8_t sckpin) {
  _cs = cspin;
  _mosi = mosipin;
  _miso = misopin;
  _sck = sckpin;

}

/*!
    @brief  Configura o Hardware.
    @return Retorna true se for bem sucedido.
*/
uint8_t ADXL372::begin() {

  digitalWrite(_cs, HIGH);
  pinMode(_cs, OUTPUT);

  if (_sck == -1) {
    /* SPI via Hardware */
    SPIinterface->begin();
  } else {
    /* SPI via Software */
    pinMode(_sck, OUTPUT);
    pinMode(_mosi, OUTPUT);
    pinMode(_miso, INPUT);
  }

  reset();

  if (readRegister(ADXL372_DEVID) != ADXL372_DEVID_VAL)
    return false;
  else
    return(ADXL372_DEVID_VAL);
}

/*!
    @brief  Exibe os bytes de todos os registros em hexadecimal.
*/
void ADXL372::registryData() {

  for (uint8_t i = 0x00; i < 0x43; i++) {
    Serial.print("0x");
    Serial.print(i, HEX);
    Serial.print(" = 0x");
    Serial.println(readRegister(i), HEX);
  }

}

/*!
    @brief  Obtêm o ID do dispositivo.
    @return Valor da ID do acelerômetro.
*/
uint8_t ADXL372::GetDeviceID() {
  return (readRegister(ADXL372_DEVID));
}


/*!
   @brief Atualiza o registro usando uma máscara.
   @param reg_addr
          Endereço do registro.
   @param mask
          Máscara a ser utilizada.
   @param data
          Valor a ser gravado no registro.
   @return 0 em caso de sucesso, código de erro negativo caso contrário.
*/
uint8_t ADXL372::Update_reg(uint8_t reg_addr, uint8_t mask, uint8_t shift, uint8_t data) {

  uint8_t reg_data;
  int err;

  err = readRegister(reg_addr);

  if (err < 0)
    return err;

  reg_data &= ~mask;
  reg_data |= (data << shift) & ~mask;

  writeRegister(reg_addr, reg_data);
  err = readRegister(reg_addr);

  //  Serial.println(err, BIN);
  return err;
}

/*!
   @brief Seta o modo de operação.
   @param ADXL372_OP_MODE mode
          Modo de operação.
*/
int ADXL372::Set_op_mode(ADXL372_OP_MODE mode) {

  return (Update_reg(ADXL372_POWER_CTL, PWRCTRL_OPMODE_MASK,
                     0, mode));
}

/*!
   @brief Seta o ODR.
   @param ADXL372_ODR odr
          ODR a ser utilizado.
*/
int ADXL372::Set_ODR(ADXL372_ODR odr) {

  return (Update_reg(ADXL372_TIMING, TIMING_ODR_MASK,
                     TIMING_ODR_POS, odr));
}

/*!
   @brief Seta modo de baixo ruído.
   @param noise
          true para low noise level, false para normal noise level.
*/
int ADXL372::Set_low_noise(bool noise) {

  return (Update_reg(ADXL372_MEASURE, MEASURE_LOW_NOISE_MSK,
                     MEASURE_LOW_NOISE_POS, noise));
}

/*!
   @brief Seta a frequência de corte do HPF.
   @param corner
          Frequência de corte do filtro.
*/
int ADXL372::Set_hpf_corner(ADXL372_HPF_CORNER corner) {

  return (Update_reg(ADXL372_HPF, HPF_CORNER_MSK,
                     HPF_CORNER_POS, corner));
}

/*!
   @brief Seta a banda do acelerômetro.
   @param bw
          Banda a ser utilizada.
*/
int ADXL372::Set_BandWidth(ADXL372_BW bw) {

  return (Update_reg(ADXL372_MEASURE, MEASURE_BANDWIDTH_MASK,
                     0, bw));
}

/*!
    @brief  Obtêm o PartID do dispositivo.
    @return Valor dp PartID do acelerômetro.
*/
uint8_t ADXL372::GetPartID() {
  return (readRegister(ADXL372_PARTID));
}

/*!
    @brief  Obtêm o RevID do dispositivo.
    @return Valor dp RevID do acelerômetro.
*/
uint8_t ADXL372::GetRevID() {
  return (readRegister(ADXL372_REVID));
}

/*!
    @brief  Obtêm o DevID do dispositivo.
    @return Valor dp DevID do acelerômetro.
*/
uint8_t ADXL372::GetDevIDMst() {
  return (readRegister(ADXL372_DEVID_MST));
}

uint8_t ADXL372::AddressToSPIData(uint8_t address, uint8_t rw_mask) {

  address = address << 1;
  return (address |= rw_mask);
}

int16_t ADXL372::ConvertFrom2Complement(uint8_t *high_part, uint8_t *low_part) {

  int16_t axis_data = 0;
  axis_data = (*high_part << 4);    // shift H part - 8MSBs to allow storing 4 LSBs
  axis_data &= 0x0FF0;              // make sure L part is zeroed
  *low_part = (*low_part  >> 4);    // shift L part to delete reserved bits
  axis_data |= *low_part;

  if ( axis_data & 0x800 ) {        // 2s complement data, MSB indicates the sign
    axis_data = ~(axis_data);
    axis_data &= 0x0FFF;            // zeroing higest part
    axis_data += 1;
    axis_data = -axis_data;
  }
  return axis_data;

}

uint8_t ADXL372::ReadAccTriplet(struct ADXL372_AccelTriplet *triplet) {

  uint8_t x_data_h = 0;
  uint8_t x_data_l = 0;
  uint8_t y_data_h = 0;
  uint8_t y_data_l = 0;
  uint8_t z_data_h = 0;
  uint8_t z_data_l = 0;
  uint8_t status1, status2;
  uint16_t fifo_entries;

  do {
    GetStatus(&status1,
              &status2, &fifo_entries);

    if ((ADXL372_STATUS_1_DATA_RDY(status1)) < 0)
      return (ADXL372_STATUS_1_DATA_RDY(status1));

  } while (!(ADXL372_STATUS_1_DATA_RDY(status1)));

  // burst transfer, auto-increment of adress
  readRegisterM(ADXL372_X_DATA_H); // ignore received data during transmit
  x_data_h = spiTransfer(0);
  x_data_l = spiTransfer(0);
  y_data_h = spiTransfer(0);
  y_data_l = spiTransfer(0);
  z_data_h = spiTransfer(0);
  z_data_l = spiTransfer(0);
  digitalWrite(_cs, HIGH);

  triplet->x = ConvertFrom2Complement(&x_data_h, &x_data_l);
  triplet->y = ConvertFrom2Complement(&y_data_h, &y_data_l);
  triplet->z = ConvertFrom2Complement(&z_data_h, &z_data_l);

  return true;

}

struct ADXL372_AccelTripletG ADXL372::ConvertAccTripletToG(const struct ADXL372_AccelTriplet *triplet) {

  struct ADXL372_AccelTripletG triplet_g = {0.0, 0.0, 0.0};
  triplet_g.x = triplet->x * 100.0 / 1000.0;
  triplet_g.y = triplet->y * 100.0 / 1000.0;
  triplet_g.z = triplet->z * 100.0 / 1000.0;
  return (triplet_g);

}

struct ADXL372_AccelTripletMs2 ADXL372::ConvertAccTripletToMs2(const struct ADXL372_AccelTriplet *triplet) {

  struct ADXL372_AccelTripletMs2 triplet_Ms2 = {0.0, 0.0, 0.0};
  triplet_Ms2.x = triplet->x * 0.980665;
  triplet_Ms2.y = triplet->y * 0.980665;
  triplet_Ms2.z = triplet->z * 0.980665;
  return (triplet_Ms2);

}

void ADXL372::SetAxisOffsets() {

  // x axis
  writeRegister(ADXL372_OFFSET_X, 0x00);
  // y axis
  writeRegister(ADXL372_OFFSET_Y, 0x00);
  // z axis
  writeRegister(ADXL372_OFFSET_Z, 0x00);

}

/*!
    @brief  Reseta o dispositivo.
*/
void ADXL372::reset() {

  Set_op_mode(STAND_BY);
  writeRegister(ADXL372_RESET, ADXL372_RESET_CODE);
  delay(10*15); // previously vTaskDelay(10), but this seems to be a FreeRTOS function? Changed to Arudino delay, assuming 1 tick ~= 15ms
}

/*!
    @brief  SPI de baixo nível.
    @param  x
            Valor que será gravado pelo SPI.
    @return reply
*/
uint8_t ADXL372::spiTransfer(uint8_t x) {
  if (_sck == -1)
    return SPIinterface->transfer(x);

  /* SPI via Software */
  uint8_t reply = 0;
  for (int i = 7; i >= 0; i--) {
    reply <<= 1;
    digitalWrite(_sck, LOW);
    digitalWrite(_mosi, x & (1 << i));
    digitalWrite(_sck, HIGH);
    if (digitalRead(_miso))
      reply |= 1;
  }
  return reply;
}

/*!
    @brief  Grava 8-bits no registro designado.
    @param  reg
            Endereço do registro.
    @param  value
            Valor que será gravado no registro.
*/
void ADXL372::writeRegister(uint8_t reg, uint8_t value) {

  if (_sck == -1)
    SPIinterface->beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));

  digitalWrite(_cs, LOW);

  spiTransfer((reg << 1 ) & ~0x01); /* Desloca os bits, seta o LSB em 0 e escreve */
  spiTransfer(value);
  digitalWrite(_cs, HIGH);
  if (_sck == -1)
    SPIinterface->endTransaction(); /* Libera o barramento do SPI */

}

/*!
    @brief  Lê 8-bits do registro designado.
    @param  reg
            Endereço do registro.
    @return Valor lido.
*/
uint8_t ADXL372::readRegister(uint8_t reg) {
  uint8_t value;

  if (_sck == -1)
    SPIinterface->beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  spiTransfer((reg << 1) | 0x01); /* Desloca os bits, seta o LSB em 1 e faz a leitura*/
  value = spiTransfer(0);
  digitalWrite(_cs, HIGH);
  if (_sck == -1)
    SPIinterface->endTransaction(); // release the SPI bus

  return value;
}

uint8_t ADXL372::readRegisterM(uint8_t reg) {
  uint8_t value;

  if (_sck == -1)
    SPIinterface->beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(_cs, LOW);
  spiTransfer((reg << 1) | 0x01); /* Desloca os bits, seta o LSB em 1 e faz a leitura*/
  //digitalWrite(_cs, HIGH);
  if (_sck == -1)
    SPIinterface->endTransaction(); // release the SPI bus

  return value;
}

void ADXL372::GetStatus(
  uint8_t *status1,
  uint8_t *status2,
  uint16_t *fifo_entries)
{
  uint8_t buf[4];
  int32_t ret;


  readRegisterM(ADXL372_STATUS_1); // ignore received data during transmit
  buf[0] = spiTransfer(0);
  buf[1] = spiTransfer(0);
  buf[2] = spiTransfer(0);
  buf[3] = spiTransfer(0);
  digitalWrite(_cs, HIGH);
  *status1 = buf[0];
  *status2 = buf[1];
  *fifo_entries = ((buf[2] & 0x3) << 8) | buf[3];

}

void ADXL372::defaultConfigurations() {

  /* Seta a banda do acelerômetro */
  Set_BandWidth(BW_3200Hz);
  /* Seta o modo de baixo ruído */
  Set_low_noise(true);
  /* Seta o ODR */
  Set_ODR(ODR_6400Hz);
  /* Seta a frequência de corte do HPF */
  Set_hpf_corner(HPF_CORNER0);
  /* Seta o modo de operação */
  Set_op_mode(FULL_BW_MEASUREMENT);
}
