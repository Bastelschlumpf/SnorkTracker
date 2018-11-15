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

#define BARO_CORR_HPA 34.5879 //!< Correction for 289m above sea level

/**
  * Communication with the BME280 modul to read temperature, humidity and pressure
  */
class MyBME280
{
protected:
   MyOptions      &myOptions;    //!< Reference to global options
   MyData         &myData;       //!< Reference to global data
   int             pinPower;     //!< Pin connection to switch on the BME280 module
   Adafruit_BME280 bme280;       //!< Adafruit BME280 helper interface
   
public:
   MyBME280(MyOptions &options, MyData &data, int pin);

   bool begin();

   bool readValues();
};

/** Constructor */
MyBME280::MyBME280(MyOptions &options, MyData &data, int pin)
   : pinPower(pin)
   , myOptions(options)
   , myData(data)
{
}

/** Switch off the module at startup to safe power. */
bool MyBME280::begin()
{
   pinMode(pinPower, OUTPUT);
   digitalWrite(pinPower, HIGH); 
}

/** 
  * Switch on the modul, read the values and switch off the modul to save power. 
  * Do this only every bme280CheckIntervalSec
  */
bool MyBME280::readValues()
{
   if (secondsElapsed(myData.rtcData.lastBme280ReadSec, myOptions.bme280CheckIntervalSec)) {
      digitalWrite(pinPower, LOW); 
      if (bme280.begin()) {
         myData.temperature = bme280.readTemperature();
         myData.humidity    = bme280.readHumidity();
         myData.pressure    = (bme280.readPressure() / 100.0F) + BARO_CORR_HPA;
      }
      digitalWrite(pinPower, HIGH); 
   }
}
