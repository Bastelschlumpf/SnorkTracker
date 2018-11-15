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
      uint32_t powerCheckCounter;    //!< Counter of the power checks from last deepsleep.
      uint32_t deepSleepCounter;     //!< Counter of the deepsleep cycles.
      uint32_t lastBme280ReadSec;    //!< Timestamp of the last BME280 read.
      uint32_t lastGpsReadSec;       //!< Timestamp of the last gps read.
      uint32_t lastMqttReconnectSec; //!< Timestamp from the last server connection. 
      uint32_t lastMqttSendSec;      //!< Timestamp from the last send.
      uint32_t crcValue;             //!< CRC of the RtcData

   public:
      RtcData()
         : powerCheckCounter(0)
         , deepSleepCounter(0)
         , lastBme280ReadSec(0)
         , lastGpsReadSec(0)
         , lastMqttReconnectSec(0)
         , lastMqttSendSec(0)
      {
         crcValue = getCRC();
      }

      uint32_t getCRC()
      {
         uint32_t crc = 0;

         crc = crc32(crc, powerCheckCounter,    sizeof(uint32_t));
         crc = crc32(crc, deepSleepCounter,     sizeof(uint32_t));
         crc = crc32(crc, lastBme280ReadSec,    sizeof(uint32_t));
         crc = crc32(crc, lastGpsReadSec,       sizeof(uint32_t));
         crc = crc32(crc, lastMqttReconnectSec, sizeof(uint32_t));
         crc = crc32(crc, lastMqttSendSec,      sizeof(uint32_t));
         return crc;
      }

      void setCRC()
      {
         crcValue = getCRC();
      }
      bool isValid()
      {
         return getCRC() == crcValue;
      }
   } rtcData;

   String status;             //!< Status information
   String restartInfo;        //!< Information on restart
   bool   isOtaActive;        //!< Is OverTheAir update active?
   long   secondsToDeepSleep; //!< Time until next deepsleep

   double voltage;            //!< Current supply voltage
   double temperature;        //!< Current BME280 temperature
   double humidity;           //!< Current BME280 humidity
   double pressure;           //!< Current BME280 pressure

   String softAPIP;           //!< registered ip of the access point
   String softAPmacAddress;   //!< module mac address
   String stationIP;          //!< registered station ip
   
   String modemInfo;          //!< Information from SIM808
   String modemIP;            //!< registered modem ip
   String imei;               //!< IMEI of the sim card
   String cop;                //!< Operator selection
   String signalQuality;      //!< Quality of the signal
   String batteryLevel;       //!< Battery level of the sim808 module
   String batteryVolt;        //!< Battery volt of the sim808 module
   
   String longitude;          //!< Current GPS longitude
   String latitude;           //!< Current GPS latitude
   String altitude;           //!< Current GPS altitude
   String kmph;               //!< Current speed in km/h
   String satellites;         //!< number of connected satellites
   String course;             //!< course of movement
   String gpsDate;            //!< Date from GPS (UTC)
   String gpsTime;            //!< Time from GPS (UTC)
   long   lastGpsUpdateSec;   //!< Elapsed Time of last read
   
   bool   isMoving;           //!< Is moving recognized
   double movingDistance;     //!< Minimum distance for moving flag
   
   StringList consoleCmds;    //!< open commands to send to the sim808 module
   StringList logInfos;       //!< received sim808 answers or other logs

public:
   MyData()
      : isOtaActive(false)
      , secondsToDeepSleep(-1)
      , voltage(0.0)
      , temperature(0.0)
      , humidity(0.0)
      , pressure(0.0)
      , isMoving(false)
      , movingDistance(0.0)
      , lastGpsUpdateSec(0)
   {
   }
};
