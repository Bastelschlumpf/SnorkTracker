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
  * @file Utils.h
  * 
  * A collection of util functions.
  */

/**
  * Conversion of the RSSI value to a quality value.
  */
String WifiGetRssiAsQuality(int rssi)
{
   int quality = 0;

   if (rssi <= -100) {
      quality = 0;
   } else if (rssi >= -50) {
      quality = 100;
   } else {
      quality = 2 * (rssi + 100);
   }
   return String(quality);
}

/**
  * Trims the data string on the left and right side every occurence of a char from chars.
  */
String Trim(const String &data, const String &chars)
{
   String ret = data;
   
   for (int i = 0; i < ret.length(); i++) {
      if (chars.indexOf(ret[i]) != -1) {
         ret.remove(i);
         i--;
         continue;
      }
      break;
   }
   for (int i = data.length() - 1; i >= 0; i--) {
      if (chars.indexOf(ret[i]) != -1) {
         ret.remove(i);
         continue;
      }
      break;
   }
   return ret;
}

/**
  * Helper function to start the OTA functionality of the ESP.
  */
void SetupOTA()
{
   MyDbg("StartOTA");
      
   ArduinoOTA.setHostname("ESP8266SIM808");

   ArduinoOTA.setPort(8266);

   ArduinoOTA.onStart([]() {
      MyDbg("OTA Start");
   });
   ArduinoOTA.onEnd([]() {
      MyDbg("\nOTA End");
   });
   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      MyDbg("OTA Progress: " + String(progress / (total / 100)));
   });   
   ArduinoOTA.onError([](ota_error_t error) {
      MyDbg("OTA Error[%u]: " + error);
      if (error == OTA_AUTH_ERROR) MyDbg("OTA Auth Failed");
      else if (error == OTA_BEGIN_ERROR) MyDbg("OTA Begin Failed");
      else if (error == OTA_CONNECT_ERROR) MyDbg("OTA Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) MyDbg("OTA Receive Failed");
      else if (error == OTA_END_ERROR) MyDbg("OTA End Failed");
   });
}
