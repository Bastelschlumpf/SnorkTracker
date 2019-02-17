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
  * @file Options.h
  *
  * Configuration data with load and save to the SPIFFS.
  */

#define OPTION_FILE_NAME "/options.txt" //!< Option file name.

/** 
  * Class with the complete configuration data of the programm.
  * It can load and save the data in a ini file format to the SPIFFS
  * as key value pairs 'key=value' line by line.
  */
class MyOptions
{
public:
   String gprsAP;                    //!< GRPS access point of the sim card supplier.
   String gprsUser;                  //!< GRPS access point User.
   String gprsPassword;              //!< GRPS access point Password.
   String wifiAP;                    //!< WiFi AP name.
   String wifiPassword;              //!< WiFi AP password.
   bool   isDebugActive;             //!< Is detailed debugging enabled?
   long   bme280CheckIntervalSec;    //!< Time interval to read the temp, hum and pressure.
   bool   gsmPower;                  //!< Is the GSM power from the DC-DC modul switched on? 
   bool   isSmsEnabled;              //!< Is the sms check functionality active?
   bool   isGpsEnabled;              //!< Is the gps part of the sim808 active?
   long   gpsTimeoutSec;             //!< Timeout for waiting for gps position.
   long   gpsCheckIntervalSec;       //!< Time interval to check the gps position.
   long   minMovingDistance;         //!< Minimum distance to accept as moving or not.
   String phoneNumber;               //!< Pone number for sms answers.
   long   smsCheckIntervalSec;       //!< SMS check intervall.
   bool   isDeepSleepEnabled;        //!< Should the system go into deepsleep if needed.
   double powerSaveModeVoltage;      //!< Minimum voltage to stay always alive.
   long   powerCheckIntervalSec;     //!< Time interval to check the power supply.
   long   activeTimeSec;             //!< Maximum alive time after deepsleep.
   long   deepSleepTimeSec;          //!< Time to stay in deep sleep (without check interrupts)
   bool   isMqttEnabled;             //!< Should the system connect to a MQTT server?
   String mqttName;                  //!< MQTT server name.
   String mqttId;                    //!< MQTT ID.
   String mqttServer;                //!< MQTT server url.
   long   mqttPort;                  //!< MQTT server port.
   String mqttUser;                  //!< MQTT user.
   String mqttPassword;              //!< MQTT password.
   long   mqttSendOnMoveEverySec;    //!< Send data interval to MQTT server on moving.
   long   mqttSendOnNonMoveEverySec; //!< Send data interval to MQTT server on non moving.

public:
   MyOptions();

   bool load();
   bool save();
};

/* ******************************************** */

MyOptions::MyOptions()
   : gprsAP(GPRS_AP)
   , gprsUser(GPRS_USER)
   , gprsPassword(GPRS_PASSWORD)
   , wifiAP(WIFI_SID)
   , wifiPassword(WIFI_PW)
   , isDebugActive(false)
   , bme280CheckIntervalSec(60)
   , gsmPower(false)
   , isSmsEnabled(false)
   , isGpsEnabled(false)
   , gpsTimeoutSec(180)
   , gpsCheckIntervalSec(60)
   , minMovingDistance(100)
   , phoneNumber(PHONE_NUMBER)
   , smsCheckIntervalSec(60)
   , isDeepSleepEnabled(false)
   , powerSaveModeVoltage(15.0)
   , powerCheckIntervalSec(300)
   , activeTimeSec(10)
   , deepSleepTimeSec(600)
   , isMqttEnabled(false)
   , mqttName(MQTT_NAME)
   , mqttId(MQTT_ID)
   , mqttServer(MQTT_SERVER)
   , mqttPort(MQTT_PORT)
   , mqttUser(MQTT_USER)
   , mqttPassword(MQTT_PASSWORD)
   , mqttSendOnMoveEverySec(600)
   , mqttSendOnNonMoveEverySec(600)
{
}

/** Load the key-value pairs from the option file into the option values. */
bool MyOptions::load()
{
   bool ret  = false;
   File file = SPIFFS.open(OPTION_FILE_NAME, "r");

   if (!file) {
      MyDbg(F("Failed to read options file"));
   } else {
      ret = true;
      while (file.available() && ret) {
         String line = file.readStringUntil('\n');
         int    idx  = line.indexOf('=');

         if (idx == -1) {
            MyDbg((String) F("Wrong option entry: ") + line);
            ret = false;
         } else {
            String key    = line.substring(0, idx);
            String value  = line.substring(idx + 1);
            long   lValue = atol(value.c_str());
            double fValue = atof(value.c_str());

            value.replace("\r", "");
            value.replace("\n", "");
            MyDbg((String) F("Load option '") + key + F("=") + value + F("'"));

            if (key == F("gprsAP")) {
               gprsAP = value;
            } else if (key == F("gprsUser")) {
               gprsUser = value;
            } else if (key == F("gprsPassword")) {
               gprsPassword = value;
            } else if (key == F("wifiAP")) {
               wifiAP = value;
            } else if (key == F("wifiPassword")) {
               wifiPassword = value;
            } else if (key == F("gsmPower")) {
               gsmPower = lValue;
            } else if (key == F("isDebugActive")) {
               isDebugActive = lValue;
            } else if (key == F("bme280CheckIntervalSec")) {
               bme280CheckIntervalSec = lValue;
            } else if (key == F("isSmsEnabled")) {
               isSmsEnabled = lValue;
            } else if (key == F("isGpsEnabled")) {
               isGpsEnabled = lValue;
            } else if (key == F("gpsTimeoutSec")) {
               gpsTimeoutSec = lValue;
            } else if (key == F("gpsCheckIntervalSec")) {
               gpsCheckIntervalSec = lValue;
            } else if (key == F("minMovingDistance")) {
               minMovingDistance = lValue;
            } else if (key == F("phoneNumber")) {
               phoneNumber = value;
            } else if (key == F("smsCheckIntervalSec")) {
               smsCheckIntervalSec = lValue;
            } else if (key == F("isDeepSleepEnabled")) {
               isDeepSleepEnabled = lValue;
            } else if (key == F("powerSaveModeVoltage")) {
               powerSaveModeVoltage = fValue;
            } else if (key == F("powerCheckIntervalSec")) {
               powerCheckIntervalSec = lValue;
            } else if (key == F("activeTimeSec")) {
               activeTimeSec = lValue;
            } else if (key == F("deepSleepTimeSec")) {
               deepSleepTimeSec = lValue;
            } else if (key == F("isMqttEnabled")) {
               isMqttEnabled = lValue;
            } else if (key == F("mqttName")) {
               mqttName = value;
            } else if (key == F("mqttId")) {
               mqttId = value;
            } else if (key == F("mqttServer")) {
               mqttServer = value;
            } else if (key == F("mqttPort")) {
               mqttPort = lValue;
            } else if (key == F("mqttUser")) {
               mqttUser = value;
            } else if (key == F("mqttPassword")) {
               mqttPassword = value;
            } else if (key == F("mqttSendOnMoveEverySec")) {
               mqttSendOnMoveEverySec = lValue;
            } else if (key == F("mqttSendOnNonMoveEverySec")) {
               mqttSendOnNonMoveEverySec = lValue;
            } else {
               MyDbg((String) F("Wrong option entry: ") + line);
               ret = false;
            }
         }
      }
      file.close();
   }
   if (ret) {
      MyDbg(F("Settings loaded"));
   }
   return ret;
}

/** Save all the options as key-value pair to the option file. */
bool MyOptions::save()
{
  File file = SPIFFS.open(OPTION_FILE_NAME, "w+");

  if (!file) {
     MyDbg("Failed to write options file");
  } else {
     file.println((String) F("gprsAP=")                    + gprsAP);
     file.println((String) F("gprsUser=")                  + gprsUser);
     file.println((String) F("gprsPassword=")              + gprsPassword);
     file.println((String) F("wifiAP=")                    + wifiAP);
     file.println((String) F("wifiPassword=")              + wifiPassword);
     file.println((String) F("gsmPower=")                  + String(gsmPower));
     file.println((String) F("isDebugActive=")             + String(isDebugActive));
     file.println((String) F("bme280CheckIntervalSec=")    + String(bme280CheckIntervalSec));
     file.println((String) F("isSmsEnabled=")              + String(isSmsEnabled));
     file.println((String) F("isGpsEnabled=")              + String(isGpsEnabled));
     file.println((String) F("gpsTimeoutSec=")             + String(gpsTimeoutSec));
     file.println((String) F("gpsCheckIntervalSec=")       + String(gpsCheckIntervalSec));
     file.println((String) F("minMovingDistance=")         + String(minMovingDistance));
     file.println((String) F("phoneNumber=")               + phoneNumber);
     file.println((String) F("smsCheckIntervalSec=")       + String(smsCheckIntervalSec));
     file.println((String) F("isDeepSleepEnabled=")        + String(isDeepSleepEnabled));
     file.println((String) F("powerSaveModeVoltage=")      + String(powerSaveModeVoltage, 1));
     file.println((String) F("powerCheckIntervalSec=")     + String(powerCheckIntervalSec));
     file.println((String) F("activeTimeSec=")             + String(activeTimeSec));
     file.println((String) F("deepSleepTimeSec=")          + String(deepSleepTimeSec));
     file.println((String) F("isMqttEnabled=")             + String(isMqttEnabled));
     file.println((String) F("mqttName=")                  + mqttName);
     file.println((String) F("mqttId=")                    + mqttId);
     file.println((String) F("mqttServer=")                + mqttServer);
     file.println((String) F("mqttPort=")                  + String(mqttPort));
     file.println((String) F("mqttUser=")                  + mqttUser);
     file.println((String) F("mqttPassword=")              + mqttPassword);
     file.println((String) F("mqttSendOnMoveEverySec=")    + String(mqttSendOnMoveEverySec));
     file.println((String) F("mqttSendOnNonMoveEverySec=") + String(mqttSendOnNonMoveEverySec));
     file.close();
     MyDbg(F("Settings saved"));
     return true;
  }
  return false;
}
