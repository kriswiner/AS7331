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
#include <RTC.h>
#include "AS7331.h"

// Ladybug pin assignments
#define myLed       13 // red

const char        *build_date = __DATE__;   // 11 characters MMM DD YYYY
const char        *build_time = __TIME__;   //  8 characters HH:MM:SS

#define I2C_BUS    Wire               // Define the I2C bus (Wire instance) you wish to use

I2Cdev             i2c_0(&I2C_BUS);   // Instantiate the I2Cdev object and point to the desired I2C bus

bool SerialDebug = true;

uint8_t seconds, minutes, hours, day, month, year;
uint8_t Seconds, Minutes, Hours, Day, Month, Year;
 
volatile bool alarmFlag = false;

// battery voltage monitor definitions
float VDDA, VBAT, VBUS, Temperature;

//AS7331 definitions
#define AS7331_intPin   8    // interrupt pin definitions 

// Specify sensor parameters //
MMODE   mmode = AS7331_CMD_MODE;  // choices are modes are CONT, CMD, SYNS, SUND
CCLK    cclk  = AS7331_1024;      // choices are 1.024, 2.048, 4.096, or 8.192 MHz
uint8_t sb    = 0x01;             // standby enabled 0x01, standby disabled 0x00                    
uint8_t breakTime = 100;          // sample time == 8 us x breakTime (0 - 255, or 0 - 2040us range), CONT or SYNX modes

uint8_t gain = 0x00; // ADCGain = 2^(11-gain), by 2s, 1 - 2048 range,  0 < gain = B max
uint8_t time = 0x08; // 2^time in ms, so 0x09 is 2^9 = 512 ms, 0 < time = F max in HEX

//sensitivities at 1.024 MHz clock
float lsbA = 304.69f / (float)(1 << (11 - gain)) / (float)(1 << time)/1024.0f;  // nW/cm^2
float lsbB = 398.44f / (float)(1 << (11 - gain)) / (float)(1 << time)/1024.0f;
float lsbC = 191.41f / (float)(1 << (11 - gain)) / (float)(1 << time)/1024.0f;

// Logic flags to keep track of device states
uint16_t tempData= 0, UVAData = 0, UVBData = 0, UVCData = 0;
float temp_C = 0;
bool AS7331_Ready_flag = false;
uint16_t status = 0;

AS7331 AS7331(&i2c_0); // instantiate AS7331 class


void setup()
{
  /* Enable USB UART */
  Serial.begin(115200);
  Serial.blockOnOverrun(false);  
  delay(4000);
  Serial.println("Serial enabled!");

  // Test the rgb led, active LOW
  pinMode(myLed, OUTPUT);
  digitalWrite(myLed, LOW);   // toggle red led on
  delay(1000);                 // wait 1 second


  pinMode(AS7331_intPin, INPUT);  // define AS7331 data ready interrupt

  /* initialize two wire bus */
  I2C_BUS.begin();                // Set master mode, default on SDA/SCL for STM32L4
  I2C_BUS.setClock(400000);       // I2C frequency at 400 kHz
  delay(1000);

  Serial.println("Scan for I2C devices:");
  i2c_0.I2Cscan();                // should detect AS7331 at 0x14 and BME280 at 0x77
  delay(1000);
  
  /* Check internal STML082 and battery power configuration */
  VDDA = STM32.getVREF();
  Temperature = STM32.getTemperature();
  
  // Internal STM32L4 functions
  Serial.print("VDDA = "); Serial.print(VDDA, 2); Serial.println(" V");
  Serial.print("STM32L4 MCU Temperature = "); Serial.println(Temperature, 2);
  Serial.println(" "); 

  // Read the AS7331 Chip ID register, this is a good test of communication
  Serial.println("AS7331 accelerometer...");
  byte AS7331_ID = AS7331.getChipID();  // Read CHIP_ID register for AS7331
  Serial.print("AS7331 "); Serial.print("I AM "); Serial.print(AS7331_ID, HEX); Serial.print(" I should be "); Serial.println(0x21, HEX);
  Serial.println(" ");
  delay(1000); 

  if(AS7331_ID == 0x21) // check if AS7331 has acknowledged
  {
   Serial.println("AS7331 is online..."); Serial.println(" ");
   
   AS7331.reset();                                                // software reset before initialization
   delay(100); 

   AS7331.setConfigurationMode();
   AS7331.powerUp();
   AS7331.init(mmode, cclk, sb, breakTime, gain, time);
   delay(100); // let sensor settle
   AS7331.setMeasurementMode();

   }
  else 
  {
   if(AS7331_ID != 0x21) Serial.println(" AS7331 not functioning!");
  }
  
  /* Set the RTC time */
  SetDefaultRTC();
  
  // set alarm to update the RTC periodically
  RTC.enableAlarm(RTC.MATCH_ANY); // alarm once a second
  RTC.attachInterrupt(alarmMatch);

  attachInterrupt(AS7331_intPin, myinthandler, RISING);  // attach data ready INT pin output of AS7331

  status = AS7331.getStatus(); // reset interrupt flag before entering main loop
  
}/* end of setup */


void loop()
{
  /* AS7331 data ready detect*/
  if(AS7331_Ready_flag)
  {
   AS7331_Ready_flag = false;    // clear the ready flag  

   status = AS7331.getStatus();

   // Error handling
   if(status & 0x0080) Serial.println("overflow of internal time reference!");
   if(status & 0x0040) Serial.println("overflow of measurement register(s)!");
   if(status & 0x0020) Serial.println("overflow of internal conversion channel(s)!");
   if(status & 0x0010) Serial.println("measurement results overwritten!");
   if(status & 0x0004) Serial.println("measurement in progress!");
  
   if(status & 0x0008) {  // when data ready
   // Serial.print("status = 0x"); Serial.println(status, HEX);

   UVAData = AS7331.readUVAData(); // read all of the data
   UVBData = AS7331.readUVBData();
   UVCData = AS7331.readUVCData();
   tempData = AS7331.readTempData();

   AS7331.powerDown(); // put the sensor back in lowest power mode until next conversion command  
   }
   
   Serial.println("Raw counts");
   Serial.print("AS7331 UVA = "); Serial.println(UVAData);  
   Serial.print("AS7331 UVB = "); Serial.println(UVBData);  
   Serial.print("AS7331 UVC = "); Serial.println(UVCData);  
   Serial.println(" ");
   
   Serial.println("Scaled UV data");
   Serial.print("AS7331 UVA (uW/cm^2)= "); Serial.println((float)(UVAData*1000)*lsbA);  
   Serial.print("AS7331 UVB (uW/cm^2)= "); Serial.println((float)(UVBData*1000)*lsbB);  
   Serial.print("AS7331 UVC (uW/cm^2)= "); Serial.println((float)(UVCData*1000)*lsbC);  
   Serial.println(" ");

   temp_C = tempData * 0.05f - 66.9f;
   Serial.print("AS7331 Temperature = "); Serial.print(temp_C, 2); Serial.println(" C");
   Serial.println(" ");

  } /* end of AS7331 data ready interrupt handling*/

 
  /*RTC*/
  if (alarmFlag) { // update RTC output at the alarm
      alarmFlag = false;

    AS7331.powerUp();  // power up the sensor
    AS7331.oneShot();  // take one UV measurement (do one conversion), interrupt when data ready
           
    VDDA = STM32.getVREF();
    Temperature = STM32.getTemperature();
    if(SerialDebug) {
      Serial.print("VDDA = "); Serial.print(VDDA, 2); Serial.println(" V");
      Serial.print("STM32L4 MCU Temperature = "); Serial.println(Temperature, 2);
      Serial.println(" ");
    }

  Serial.println("RTC:");
  Day = RTC.getDay();
  Month = RTC.getMonth();
  Year = RTC.getYear();
  Seconds = RTC.getSeconds();
  Minutes = RTC.getMinutes();
  Hours   = RTC.getHours();     
  if(Hours < 10) {Serial.print("0"); Serial.print(Hours);} else Serial.print(Hours);
  Serial.print(":"); 
  if(Minutes < 10) {Serial.print("0"); Serial.print(Minutes);} else Serial.print(Minutes); 
  Serial.print(":"); 
  if(Seconds < 10) {Serial.print("0"); Serial.println(Seconds);} else Serial.println(Seconds);  

  Serial.print(Month); Serial.print("/"); Serial.print(Day); Serial.print("/"); Serial.println(Year);
  Serial.println(" ");
  
    digitalWrite(myLed, HIGH); delay(1);  digitalWrite(myLed, LOW); // toggle red led on
 } // end of RTC alarm section
    
//    STM32.stop();       // Enter STOP mode and wait for an interrupt
    STM32.sleep();        // Enter SLEEP mode and wait for an interrupt
   
}  /* end of loop*/


/* Useful functions */
void myinthandler()
{
  AS7331_Ready_flag = true; 
}


void alarmMatch()
{
  alarmFlag = true;
}


void SetDefaultRTC()                                                                                 // Function sets the RTC to the FW build date-time...
{
  char Build_mo[3];
  String build_mo = "";

  Build_mo[0] = build_date[0];                                                                       // Convert month string to integer
  Build_mo[1] = build_date[1];
  Build_mo[2] = build_date[2];
  for(uint8_t i=0; i<3; i++)
  {
    build_mo += Build_mo[i];
  }
  if(build_mo == "Jan")
  {
    month = 1;
  } else if(build_mo == "Feb")
  {
    month = 2;
  } else if(build_mo == "Mar")
  {
    month = 3;
  } else if(build_mo == "Apr")
  {
    month = 4;
  } else if(build_mo == "May")
  {
    month = 5;
  } else if(build_mo == "Jun")
  {
    month = 6;
  } else if(build_mo == "Jul")
  {
    month = 7;
  } else if(build_mo == "Aug")
  {
    month = 8;
  } else if(build_mo == "Sep")
  {
    month = 9;
  } else if(build_mo == "Oct")
  {
    month = 10;
  } else if(build_mo == "Nov")
  {
    month = 11;
  } else if(build_mo == "Dec")
  {
    month = 12;
  } else
  {
    month = 1;                                                                                       // Default to January if something goes wrong...
  }
  if(build_date[4] != 32)                                                                            // If the first digit of the date string is not a space
  {
    day   = (build_date[4] - 48)*10 + build_date[5]  - 48;                                           // Convert ASCII strings to integers; ASCII "0" = 48
  } else
  {
    day   = build_date[5]  - 48;
  }
  year    = (build_date[9] - 48)*10 + build_date[10] - 48;
  hours   = (build_time[0] - 48)*10 + build_time[1]  - 48;
  minutes = (build_time[3] - 48)*10 + build_time[4]  - 48;
  seconds = (build_time[6] - 48)*10 + build_time[7]  - 48;
  RTC.setDay(day);                                                                                   // Set the date/time
  RTC.setMonth(month);
  RTC.setYear(year);
  RTC.setHours(hours);
  RTC.setMinutes(minutes);
  RTC.setSeconds(seconds);
}
