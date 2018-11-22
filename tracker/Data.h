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
  * @file Data.h
  * 
  * Class with all the global runtime data.
  */


/**
  * Helper class to store all the global determined data in one place.
  */
class MyData
{
public:
   class RtcData {
   public:
      MyLocation lastLocation;           //!< Last known GPS position. 

      long       aktiveTimeSec;          //!< Time in active mode without current millis().
      long       powerOnTimeSec;         //!< Time the sim808 is on power without current millis..
      long       deepSleepTimeSec;       //!< Time in deep sleep mode. 
      long       deepSleepStartSec;      //!< Timestamp of the last deep sleep start.
                 
      long       lowPowerActiveTimeSec;  //!< Timestamp of the last deep sleep start.
      long       lowPowerPowerOnTimeSec; //!< Timestamp of the last deep sleep start.
                 
      long       lastBme280ReadSec;      //!< Timestamp of the last BME280 read.
      long       lastSmsCheckSec;        //!< Timestamp of the last sms check.
      long       lastGpsReadSec;         //!< Timestamp of the last gps read.
      long       lastMqttSendSec;        //!< Timestamp from the last send.
                 
      long       crcValue;               //!< CRC of the RtcData

   public:
      RtcData();

      bool isValid();
      void reset();
      void setCRC();
      long getCRC();
   } rtcData;

   String status;              //!< Status information
   String restartInfo;         //!< Information on restart
   bool   isOtaActive;         //!< Is OverTheAir update active?
   bool   isPowerOn;           //!< Is the power of the sim808 switched on?
   bool   isLowPower;          //!< Is the power below min voltage?

   long   secondsToDeepSleep;  //!< Time until next deepsleep. -1 = disabled
   long   awakeTimeOffsetSec;  //!< Awake time offset for SaveSettings.

   double voltage;             //!< Current supply voltage
   double temperature;         //!< Current BME280 temperature
   double humidity;            //!< Current BME280 humidity
   double pressure;            //!< Current BME280 pressure

   String softAPIP;            //!< registered ip of the access point
   String softAPmacAddress;    //!< module mac address
   String stationIP;           //!< registered station ip
   
   String modemInfo;           //!< Information from SIM808
   String modemIP;             //!< registered modem ip
   String imei;                //!< IMEI of the sim card
   String cop;                 //!< Operator selection
   String signalQuality;       //!< Quality of the signal
   String batteryLevel;        //!< Battery level of the sim808 module
   String batteryVolt;         //!< Battery volt of the sim808 module
   
   MyGps  gps;                 //!< Current GPS data. 
   long   lastGpsUpdateSec;    //!< Elapsed Time of last read
   
   bool   isMoving;            //!< Is moving recognized
   double movingDistance;      //!< Minimum distance for moving flag
   
   StringList consoleCmds;     //!< open commands to send to the sim808 module
   StringList logInfos;        //!< received sim808 answers or other logs

public:
   MyData();

   long   secondsSincePowerOn();
   long   getActiveTimeSec();
   long   getLowPowerActiveTimeSec();
   long   getPowerOnTimeSec();
   long   getLowPowerPowerOnTimeSec();

   double getPowerConsumption();
   double getLowPowerPowerConsumption();
};

/* ******************************************** */

MyData::RtcData::RtcData()
   : aktiveTimeSec(0)
   , powerOnTimeSec(0)
   , deepSleepTimeSec(0)
   , deepSleepStartSec(0)
   , lowPowerActiveTimeSec(0)
   , lowPowerPowerOnTimeSec(0)
   , lastBme280ReadSec(0)
   , lastSmsCheckSec(0)
   , lastGpsReadSec(0)
   , lastMqttSendSec(0)
{
   crcValue = getCRC();
}

/** Does the CRC fit s the content */
bool MyData::RtcData::isValid()
{
   return getCRC() == crcValue;
}

/** Creates the CRC of all the data and save it in the class. */
void MyData::RtcData::setCRC()
{
   crcValue = getCRC();
}

/** Creates a CRC of all the member variables. */
long MyData::RtcData::getCRC()
{
   long crc = 0;

   crc = crc32(crc, (unsigned char *) &lastLocation,           sizeof(MyLocation));
   crc = crc32(crc, (unsigned char *) &aktiveTimeSec,          sizeof(long));
   crc = crc32(crc, (unsigned char *) &powerOnTimeSec,         sizeof(long));
   crc = crc32(crc, (unsigned char *) &deepSleepTimeSec,       sizeof(long));
   crc = crc32(crc, (unsigned char *) &deepSleepStartSec,      sizeof(long));
   crc = crc32(crc, (unsigned char *) &lowPowerActiveTimeSec,  sizeof(long));
   crc = crc32(crc, (unsigned char *) &lowPowerPowerOnTimeSec, sizeof(long));
   crc = crc32(crc, (unsigned char *) &lastBme280ReadSec,      sizeof(long));
   crc = crc32(crc, (unsigned char *) &lastSmsCheckSec,        sizeof(long));
   crc = crc32(crc, (unsigned char *) &lastGpsReadSec,         sizeof(long));
   crc = crc32(crc, (unsigned char *) &lastMqttSendSec,        sizeof(long));
   return crc;
}

/** Constructor */
MyData::MyData()
   : isOtaActive(false)
   , isPowerOn(false)
   , isLowPower(false)
   , secondsToDeepSleep(-1)
   , awakeTimeOffsetSec(0)
   , voltage(0.0)
   , temperature(0.0)
   , humidity(0.0)
   , pressure(0.0)
   , isMoving(false)
   , movingDistance(0.0)
   , lastGpsUpdateSec(0)
{
}

/** Returns the seconds since power up (not since last deep sleep). */
long MyData::secondsSincePowerOn()
{
   return rtcData.deepSleepTimeSec + getActiveTimeSec();
}

/** Return all the active over all deep sleeps plus the current active time. */
long MyData::getActiveTimeSec()
{
   return rtcData.aktiveTimeSec + millis() / 1000;
}

/** Return all the active over all deep sleeps plus the current active time. */
long MyData::getLowPowerActiveTimeSec()
{
   if (!isLowPower) {
      return rtcData.lowPowerActiveTimeSec;
   } else {
      return rtcData.lowPowerActiveTimeSec + millis() / 1000;
   }
}

/** Returns the power on time over all deep sleeps plus the current active time if power is on. */
long MyData::getPowerOnTimeSec()
{
   if (!isPowerOn) {
      return rtcData.powerOnTimeSec;
   } else {
      return rtcData.powerOnTimeSec + millis() / 1000;
   }
}

/** Return all the time with power on on low power supply. */
long MyData::getLowPowerPowerOnTimeSec()
{
   if (!isLowPower || !isPowerOn) {
      return rtcData.lowPowerPowerOnTimeSec;
   } else {
      return rtcData.lowPowerPowerOnTimeSec + millis() / 1000;
   }
}

/** Calculates the power consumption from power on.
  * In mA/h
  */
double MyData::getPowerConsumption()
{
   return (POWER_CONSUMPTION_ACTIVE     * (getActiveTimeSec() - getPowerOnTimeSec()) +
           POWER_CONSUMPTION_POWER_ON   * getPowerOnTimeSec() +
           POWER_CONSUMPTION_DEEP_SLEEP * rtcData.deepSleepTimeSec) / 3600.0;
}

/** Calculates the power consumption on low power from power on.
  * In mA/h
  */
double MyData::getLowPowerPowerConsumption()
{
   return (POWER_CONSUMPTION_ACTIVE     * (getLowPowerActiveTimeSec() - getLowPowerPowerOnTimeSec()) +
           POWER_CONSUMPTION_POWER_ON   * getLowPowerPowerOnTimeSec() +
           POWER_CONSUMPTION_DEEP_SLEEP * rtcData.deepSleepTimeSec) / 3600.0;
}
