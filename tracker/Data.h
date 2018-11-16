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
      uint32_t aktiveTimeSec;        //!< Time in active mode without current millis().
      uint32_t powerOnTimeSec;       //!< Time the sim808 is on power without current millis..
      uint32_t deepSleepTimeSec;     //!< Time in deep sleep mode. 
      uint32_t deepSleepStartSec;    //!< Timestamp of the last deep sleep start.

      uint32_t lastBme280ReadSec;    //!< Timestamp of the last BME280 read.
      uint32_t lastGpsReadSec;       //!< Timestamp of the last gps read.
      uint32_t lastMqttReconnectSec; //!< Timestamp from the last server connection. 
      uint32_t lastMqttSendSec;      //!< Timestamp from the last send.

      uint32_t crcValue;             //!< CRC of the RtcData

   public:
      RtcData();

      bool     isValid();
      void     reset();
      void     setCRC();
      uint32_t getCRC();
   } rtcData;

   String   status;              //!< Status information
   String   restartInfo;         //!< Information on restart
   bool     isOtaActive;         //!< Is OverTheAir update active?
   bool     isPowerOn;           //!< Is the power of the sim808 switched on?

   int32_t  secondsToDeepSleep;  //!< Time until next deepsleep. -1 = disabled
   uint32_t awakeTimeOffsetSec;  //!< Awake time offset for SaveSettings.

   double   voltage;             //!< Current supply voltage
   double   temperature;         //!< Current BME280 temperature
   double   humidity;            //!< Current BME280 humidity
   double   pressure;            //!< Current BME280 pressure

   String   softAPIP;            //!< registered ip of the access point
   String   softAPmacAddress;    //!< module mac address
   String   stationIP;           //!< registered station ip
   
   String   modemInfo;           //!< Information from SIM808
   String   modemIP;             //!< registered modem ip
   String   imei;                //!< IMEI of the sim card
   String   cop;                 //!< Operator selection
   String   signalQuality;       //!< Quality of the signal
   String   batteryLevel;        //!< Battery level of the sim808 module
   String   batteryVolt;         //!< Battery volt of the sim808 module
   
   String   longitude;           //!< Current GPS longitude
   String   latitude;            //!< Current GPS latitude
   String   altitude;            //!< Current GPS altitude
   String   kmph;                //!< Current speed in km/h
   String   satellites;          //!< number of connected satellites
   String   course;              //!< course of movement
   String   gpsDate;             //!< Date from GPS (UTC)
   String   gpsTime;             //!< Time from GPS (UTC)
   uint32_t lastGpsUpdateSec;    //!< Elapsed Time of last read
   
   bool     isMoving;            //!< Is moving recognized
   double   movingDistance;      //!< Minimum distance for moving flag
   
   StringList consoleCmds;       //!< open commands to send to the sim808 module
   StringList logInfos;          //!< received sim808 answers or other logs

public:
   MyData();

   uint32_t secondsSincePowerOn();
   uint32_t getActiveTimeSec();
   uint32_t getPowerOnTimeSec();

   double   getPowerConsumption();
};

/* ******************************************** */

MyData::RtcData::RtcData()
   : aktiveTimeSec(0)
   , powerOnTimeSec(0)
   , deepSleepTimeSec(0)
   , deepSleepStartSec(0)
   , lastBme280ReadSec(0)
   , lastGpsReadSec(0)
   , lastMqttReconnectSec(0)
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
uint32_t MyData::RtcData::getCRC()
{
   uint32_t crc = 0;

   crc = crc32(crc, aktiveTimeSec,        sizeof(uint32_t));
   crc = crc32(crc, powerOnTimeSec,       sizeof(uint32_t));
   crc = crc32(crc, deepSleepTimeSec,     sizeof(uint32_t));
   crc = crc32(crc, deepSleepStartSec,    sizeof(uint32_t));
   crc = crc32(crc, lastBme280ReadSec,    sizeof(uint32_t));
   crc = crc32(crc, lastGpsReadSec,       sizeof(uint32_t));
   crc = crc32(crc, lastMqttReconnectSec, sizeof(uint32_t));
   crc = crc32(crc, lastMqttSendSec,      sizeof(uint32_t));
   return crc;
}

/** Constructor */
MyData::MyData()
   : isOtaActive(false)
   , isPowerOn(false)
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
uint32_t MyData::secondsSincePowerOn()
{
   return rtcData.deepSleepTimeSec + rtcData.aktiveTimeSec + millis() / 1000;
}

/** Return all the active over all deep sleeps plus the current active time. */
uint32_t MyData::getActiveTimeSec()
{
   return rtcData.aktiveTimeSec + millis() / 1000;
}

/** Returns the power on time over all deep sleeps plus the current active time if power is on. */
uint32_t MyData::getPowerOnTimeSec()
{
   if (!isPowerOn) {
      return rtcData.powerOnTimeSec;
   } else {
      return rtcData.powerOnTimeSec + millis() / 1000;
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
