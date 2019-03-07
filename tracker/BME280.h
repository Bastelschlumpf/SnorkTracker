/*
   Copyright (C) 2018 SFini

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
  * @file BME280.h
  * 
  * Class to communicate with the BME280 module.
  */


#include <Adafruit_BME280.h>

#define BARO_CORR_HPA     34.5879 //!< Correction for 289m above sea level
#define TEMP_CORR_DEGREE  -2.0    //!< The BME280 measure 2 degrees too high 

/**
  * Communication with the BME280 modul to read temperature, humidity and pressure
  */
class MyBME280
{
protected:
   MyOptions      &myOptions;    //!< Reference to global options
   MyData         &myData;       //!< Reference to global data
   int             pinGrnd;      //!< Ground-Pin connection to switch on the BME280 module
   uint8_t         portAddr;     //!< Port address of the bme280
   Adafruit_BME280 bme280;       //!< Adafruit BME280 helper interface
   
public:
   MyBME280(MyOptions &options, MyData &data, int pin, uint8_t addr);

   bool begin();

   bool readValues();
};

/* ******************************************** */

/** Constructor */
MyBME280::MyBME280(MyOptions &options, MyData &data, int pin, uint8_t addr)
   : pinGrnd(pin)
   , myOptions(options)
   , myData(data)
   , portAddr(addr)
{
}

/** Switch off the module at startup to safe power. */
bool MyBME280::begin()
{
   pinMode(D1,      INPUT); // I2C SCL Open state to safe power
   pinMode(D2,      INPUT); // I2C SDA Open state to safe power
   pinMode(pinGrnd, OUTPUT);
   digitalWrite(pinGrnd, HIGH); 
}

/** 
  * Switch on the modul, read the values and switch off the modul to save power. 
  * Do this only every bme280CheckIntervalSec
  */
bool MyBME280::readValues()
{
   if (secondsElapsedAndUpdate(myData.rtcData.lastBme280ReadSec, myOptions.bme280CheckIntervalSec)) {
      digitalWrite(pinGrnd, LOW);
      delay(100); // Short delay after power on
      if (!bme280.begin(portAddr)) {
         myData.temperature = 0;
         myData.humidity    = 0;
         myData.pressure    = 0;
         MyDbg("No valid BME280 sensor, check wiring!");
      } else {
         myData.temperature = bme280.readTemperature() + TEMP_CORR_DEGREE;
         myData.humidity    = bme280.readHumidity();
         myData.pressure    = (bme280.readPressure() / 100.0F) + BARO_CORR_HPA;
         MyDbg("Temperature: " + String(myData.temperature) + "°C");
         MyDbg("Humidity: "    + String(myData.humidity)    + "%");
         MyDbg("Pressure: "    + String(myData.pressure)    + "hPa");
      }
      digitalWrite(pinGrnd, HIGH); 
      pinMode(D1, INPUT); // I2C SCL Open state to safe power
      pinMode(D2, INPUT); // I2C SDA Open state to safe power
   }
}
