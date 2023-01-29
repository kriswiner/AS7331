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
#include "AS7331.h"
#include "I2CDev.h"

AS7331::AS7331(I2Cdev* i2c_bus)
{
  _i2c_bus = i2c_bus;
}


  uint8_t AS7331::getChipID()
  {
  uint8_t c = _i2c_bus->readByte(AS7331_ADDRESS, AS7331_AGEN);
  return c;
  }


  void AS7331::powerDown() 
  {
  uint8_t temp = _i2c_bus->readByte(AS7331_ADDRESS, AS7331_OSR);    // Read the raw data register  
  _i2c_bus->writeByte(AS7331_ADDRESS, AS7331_OSR, temp & ~(0x40) ); // clear bit 6
  }


  void AS7331::powerUp() 
  {
  uint8_t temp = _i2c_bus->readByte(AS7331_ADDRESS, AS7331_OSR);  // Read the raw data register  
  _i2c_bus->writeByte(AS7331_ADDRESS, AS7331_OSR, temp | 0x40);   // set bit 6
  }


  void AS7331::reset()
 {
  uint8_t temp = _i2c_bus->readByte(AS7331_ADDRESS, AS7331_OSR);  
  _i2c_bus->writeByte(AS7331_ADDRESS, AS7331_OSR, temp | 0x08); // set bit 3 for software reset the AS7331
 }


  void AS7331::setConfigurationMode()
 {
  uint8_t temp = _i2c_bus->readByte(AS7331_ADDRESS, AS7331_OSR);  
  _i2c_bus->writeByte(AS7331_ADDRESS, AS7331_OSR, temp | 0x02); // set bit 1 for configuration mode
 }


 void AS7331::setMeasurementMode()
 {
  uint8_t temp = _i2c_bus->readByte(AS7331_ADDRESS, AS7331_OSR);  
  _i2c_bus->writeByte(AS7331_ADDRESS, AS7331_OSR, temp | 0x03); // set bits 0,1 for measurement mode
 }


  void AS7331::init(uint8_t mmode, uint8_t cclk, uint8_t sb, uint8_t breakTime, uint8_t gain, uint8_t time)
 {
//   set measurement mode (bits 6,7), standby on/off (bit 4)
//   and internal clk (bits 0,1); bit 3 determines ready interrupt configuration, 0 means push pull
//   1 means open drain
   _i2c_bus->writeByte(AS7331_ADDRESS, AS7331_CREG1, gain << 4 |  time ); //  
   _i2c_bus->writeByte(AS7331_ADDRESS, AS7331_CREG3, mmode << 6 | sb << 4 | cclk ); //  
   _i2c_bus->writeByte(AS7331_ADDRESS, AS7331_BREAK, breakTime ); //  
 }


  void AS7331::oneShot()
 {
  uint8_t temp = _i2c_bus->readByte(AS7331_ADDRESS, AS7331_OSR);  
  _i2c_bus->writeByte(AS7331_ADDRESS, AS7331_OSR, temp | 0x80); // set bit 7 for forced one-time measurement
 }


  uint16_t AS7331::getStatus()
  {
   uint8_t rawData[2];  // 16-bit status register data stored here
   _i2c_bus->readBytes(AS7331_ADDRESS, AS7331_STATUS, 2, &rawData[0]);
   // first byte for OSR information and the second byte for STATUS information   
   return (uint16_t)(((uint16_t)rawData[0]) << 8 | rawData[1]);
  }


  uint16_t AS7331::readTempData()
  {
   uint8_t rawData[2];  // 16-bit status register data stored here
   _i2c_bus->readBytes(AS7331_ADDRESS, AS7331_TEMP, 2, &rawData[0]);
   return (uint16_t)(((uint16_t)rawData[1]) << 8 | rawData[0]);
  }


  uint16_t AS7331::readUVAData()
  {
   uint8_t rawData[2];  // 16-bit status register data stored here
   _i2c_bus->readBytes(AS7331_ADDRESS, AS7331_MRES1, 2, &rawData[0]);
   return (uint16_t)(((uint16_t)rawData[1]) << 8 | rawData[0]);
  }


  uint16_t AS7331::readUVBData()
  {
   uint8_t rawData[2];  // 16-bit status register data stored here
   _i2c_bus->readBytes(AS7331_ADDRESS, AS7331_MRES2, 2, &rawData[0]);
   return (uint16_t)(((uint16_t)rawData[1]) << 8 | rawData[0]);
  }


  uint16_t AS7331::readUVCData()
  {
   uint8_t rawData[2];  // 16-bit status register data stored here
   _i2c_bus->readBytes(AS7331_ADDRESS, AS7331_MRES3, 2, &rawData[0]);
   return (uint16_t)(((uint16_t)rawData[1]) << 8 | rawData[0]);
  }

 

  
