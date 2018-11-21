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
   String gprsAP;                        //!< GRPS access point of the sim card supplier.
   String wlanAP;                        //!< WLAN AP name.
   String wlanPassword;                  //!< WLAN AR password.
   bool   isDebugActive;                 //!< Is detailed debugging enabled?
   long   bme280CheckIntervalSec;        //!< Time interval to read the temp, hum and pressure.
   bool   gsmPower;                      //!< Is the GSM power from the DC-DC modul switched on? 
   bool   isGsmEnabled;                  //!< Is the gsm part of the sim808 active?
   bool   isGpsEnabled;                  //!< Is the gps part of the sim808 active?
   long   gpsCheckIntervalSec;           //!< Time interval to check the gps position.
   long   minMovingDistance;             //!< Minimum distance to accept as moving or not.
   String phoneNumber;                   //!< Pone number for sms answers.
   long   smsCheckIntervalSec;           //!< SMS check intervall.
   bool   isDeepSleepEnabled;            //!< Should the system go into deepsleep if needed.
   double powerSaveModeVoltage;          //!< Minimum voltage to stay always alive.
   long   powerCheckIntervalSec;         //!< Time interval to check the power supply.
   long   activeTimeSec;                 //!< Maximum alive time after deepsleep.
   long   deepSleepTimeSec;              //!< Time to stay in deep sleep (without check interrupts)
   bool   isMqttEnabled;                 //!< Should the system connect to a MQTT server?
   String mqttName;                      //!< MQTT server name.
   String mqttId;                        //!< MQTT ID.
   String mqttServer;                    //!< MQTT server url.
   long   mqttPort;                      //!< MQTT server port.
   String mqttUser;                      //!< MQTT user.
   String mqttPassword;                  //!< MQTT password.
   long   mqttReconnectIntervalSec;      //!< Reconnect interval on disconnection.
   long   mqttSendOnMoveEverySec;        //!< Send data interval to MQTT server on moving.
   long   mqttSendOnNonMoveEverySec;     //!< Send data interval to MQTT server on non moving.

public:
   MyOptions();

   bool load();
   bool save();
};

/* ******************************************** */

MyOptions::MyOptions()
   : gprsAP(GPRS_AP)
   , wlanAP(WLAN_SID)
   , wlanPassword(WLAN_PW)
   , isDebugActive(false)
   , bme280CheckIntervalSec(60)
   , gsmPower(true)
   , isGsmEnabled(true)
   , isGpsEnabled(true)
   , gpsCheckIntervalSec(60)
   , minMovingDistance(100)
   , phoneNumber(PHONE_NUMBER)
   , smsCheckIntervalSec(60)
   , isDeepSleepEnabled(true)
   , powerSaveModeVoltage(15.0)
   , powerCheckIntervalSec(60)
   , activeTimeSec(10)
   , deepSleepTimeSec(600)
   , isMqttEnabled(false)
   , mqttName(MQTT_NAME)
   , mqttId(MQTT_ID)
   , mqttServer(MQTT_SERVER)
   , mqttPort(MQTT_PORT)
   , mqttUser(MQTT_USER)
   , mqttPassword(MQTT_PASSWORD)
   , mqttReconnectIntervalSec(10)
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
      MyDbg("Failed to read options file");
   } else {
      ret = true;
      while (file.available() && ret) {
         String line = file.readStringUntil('\n');
         int    idx  = line.indexOf('=');

         if (idx == -1) {
            MyDbg("Wrong option entry: " + line);
            ret = false;
         } else {
            String key    = line.substring(0, idx);
            String value  = line.substring(idx + 1);
            long   lValue = atoi(value.c_str());
            double fValue = atof(value.c_str());

            value.replace("\r", "");
            value.replace("\n", "");
            MyDbg("Load option '" + key + "=" + value + "'");

            if (key == "gprsAP") {
               gprsAP = value;
            } else if (key == "wlanAP") {
               wlanAP = value;
            } else if (key == "wlanPassword") {
               wlanPassword = value;
            } else if (key == "gsmPower") {
               gsmPower = lValue;
            } else if (key == "isDebugActive") {
               isDebugActive = lValue;
            } else if (key == "bme280CheckIntervalSec") {
               bme280CheckIntervalSec = lValue;
            } else if (key == "isGsmEnabled") {
               isGsmEnabled = lValue;
            } else if (key == "isGpsEnabled") {
               isGpsEnabled = lValue;
            } else if (key == "gpsCheckIntervalSec") {
               gpsCheckIntervalSec = lValue;
            } else if (key == "minMovingDistance") {
               minMovingDistance = lValue;
            } else if (key == "phoneNumber") {
               phoneNumber = value;
            } else if (key == "smsCheckIntervalSec") {
               smsCheckIntervalSec = lValue;
            } else if (key == "isDeepSleepEnabled") {
               isDeepSleepEnabled = lValue;
            } else if (key == "powerSaveModeVoltage") {
               powerSaveModeVoltage = fValue;
            } else if (key == "powerCheckIntervalSec") {
               powerCheckIntervalSec = lValue;
            } else if (key == "activeTimeSec") {
               activeTimeSec = lValue;
            } else if (key == "deepSleepTimeSec") {
               deepSleepTimeSec = lValue;
            } else if (key == "isMqttEnabled") {
               isMqttEnabled = lValue;
            } else if (key == "mqttName") {
               mqttName = value;
            } else if (key == "mqttId") {
               mqttId = value;
            } else if (key == "mqttServer") {
               mqttServer = value;
            } else if (key == "mqttPort") {
               mqttPort = lValue;
            } else if (key == "mqttUser") {
               mqttUser = value;
            } else if (key == "mqttPassword") {
               mqttPassword = value;
            } else if (key == "mqttSendOnMoveEverySec") {
               mqttSendOnMoveEverySec = lValue;
            } else if (key == "mqttSendOnNonMoveEverySec") {
               mqttSendOnNonMoveEverySec = lValue;
            } else {
               MyDbg("Wrong option entry: " + line);
               ret = false;
            }
         }
      }
      file.close();
   }
   if (ret) {
      MyDbg("Settings loaded");
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
     file.println("gprsAP="                    + gprsAP);
     file.println("wlanAP="                    + wlanAP);
     file.println("wlanPassword="              + wlanPassword);
     file.println("gsmPower="                  + String(gsmPower));
     file.println("isDebugActive="             + String(isDebugActive));
     file.println("bme280CheckIntervalSec="    + String(bme280CheckIntervalSec));
     file.println("isGsmEnabled="              + String(isGsmEnabled));
     file.println("isGpsEnabled="              + String(isGpsEnabled));
     file.println("gpsCheckIntervalSec="       + String(gpsCheckIntervalSec));
     file.println("minMovingDistance="         + String(minMovingDistance));
     file.println("phoneNumber="               + phoneNumber);
     file.println("smsCheckIntervalSec="       + String(smsCheckIntervalSec));
     file.println("isDeepSleepEnabled="        + String(isDeepSleepEnabled));
     file.println("powerSaveModeVoltage="      + String(powerSaveModeVoltage, 1));
     file.println("powerCheckIntervalSec="     + String(powerCheckIntervalSec));
     file.println("activeTimeSec="             + String(activeTimeSec));
     file.println("deepSleepTimeSec="          + String(deepSleepTimeSec));
     file.println("isMqttEnabled="             + String(isMqttEnabled));
     file.println("mqttName="                  + mqttName);
     file.println("mqttId="                    + mqttId);
     file.println("mqttServer="                + mqttServer);
     file.println("mqttPort="                  + String(mqttPort));
     file.println("mqttUser="                  + mqttUser);
     file.println("mqttPassword="              + mqttPassword);
     file.println("mqttSendOnMoveEverySec="    + String(mqttSendOnMoveEverySec));
     file.println("mqttSendOnNonMoveEverySec=" + String(mqttSendOnNonMoveEverySec));
     file.close();
     MyDbg("Settings saved");
     return true;
  }
  return false;
}
