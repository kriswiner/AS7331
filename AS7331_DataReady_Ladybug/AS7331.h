/* 1/28/23 Copyright Tlera Corporation
 *  
 *  Created by Kris Winer   
 *  
 *  The AS7331 is a three-channel UV light sensor with separate photodiodes sensitive to UVA, UVB, and UVC
 *  light radiation.
 *  
 *  Library may be used freely and without limit with attribution.
 *  
 */
 
#ifndef AS7331_h
#define AS7331_h

#include "Arduino.h"
#include "I2CDev.h"
#include <Wire.h>

/*
 * https://www.mouser.com/catalog/specsheets/amsOsram_AS7331_DS001047_1-00.pdf
*/
// Configuration State registers
#define AS7331_OSR                      0x00
#define AS7331_AGEN                     0x02  // should be 0x21
#define AS7331_CREG1                    0x06   
#define AS7331_CREG2                    0x07
#define AS7331_CREG3                    0x08
#define AS7331_BREAK                    0x09
#define AS7331_EDGES                    0x0A
#define AS7331_OPTREG                   0x0B

// Measurement State registers
#define AS7331_STATUS                   0x00
#define AS7331_TEMP                     0x01
#define AS7331_MRES1                    0x02
#define AS7331_MRES2                    0x03
#define AS7331_MRES3                    0x04
#define AS7331_OUTCONVL                 0x05
#define AS7331_OUTCONVH                 0x06


#define AS7331_ADDRESS  0x74  // if A0 = A1 = LOW

typedef enum {
  AS7331_CONT_MODE                = 0x00, // continuous mode
  AS7331_CMD_MODE                 = 0x01, // force mode, one-time measurement
  AS7331_SYNS_MODE                = 0x02,
  AS7331_SYND_MODE                = 0x03
} MMODE;


typedef enum  {
  AS7331_1024           = 0x00, // internal clock frequency, 1.024 MHz, etc
  AS7331_2048           = 0x01,
  AS7331_4096           = 0x02,
  AS7331_8192           = 0x03
} CCLK;


class AS7331
{
  public: 
  AS7331(I2Cdev* i2c_bus);
  uint8_t getChipID();
  void powerDown();
  void powerUp();
  void reset();
  void setMeasurementMode();
  void setConfigurationMode();
  void init(uint8_t mmode, uint8_t cclk, uint8_t sb, uint8_t breakTime, uint8_t gain, uint8_t time);
  void oneShot();
  uint16_t readTempData();
  uint16_t readUVAData();
  uint16_t readUVBData();
  uint16_t readUVCData();
  void     readAllData(uint16_t * dest);
  uint16_t getStatus();

  private:
  I2Cdev* _i2c_bus;
};

#endif
