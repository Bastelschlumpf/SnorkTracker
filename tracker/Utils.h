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
  * A collection of utility functions.
  */


/**
   * Helper class for debugging via Serial.println 
   */
class SerialOut
{
protected:
   String message; //!< Mesage on contructor and destructor.

public:
   SerialOut(String msg)
      : message(msg)
   {
      String m = ":" + String(millis()) + "[" + message;
      Serial.println(m.c_str());
   }
   ~SerialOut()
   {
      String m = message + ":" + String(millis()) + "]";
      Serial.println(m.c_str());
   }
};

/** This function has to be overwritten to return the seconds since power up (not since last deep sleep). */
long secondsSincePowerOn();

/** Checks if the intervalSec is from the last checkIntervalSec elapsed and if true it sets the lastCheckSec value */
bool secondsElapsed(long &lastCheckSec, const long &intervalSec)
{
   long currentSec = secondsSincePowerOn();

   if (lastCheckSec == 0 || (currentSec - lastCheckSec > intervalSec)) {
      lastCheckSec = currentSec;
      return true;
   }
   return false;
}

#define POLY 0xedb88320 //!< CRC-32 (Ethernet, ZIP, etc.) polynomial in reversed bit order.

/** Simple crc function. Can multiple called but the first time crc should be 0.  */
long crc32(long crc, unsigned char *buf, size_t len)
{
   crc = ~crc;
   while (len--) {
      crc ^= *buf++;
      for (int k = 0; k < 8; k++)
         crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
   }
   return ~crc;
}

/** This function has to be overwritten to implement the handle of debug informations. */
void myDebugInfo(String info, bool isWebServer, bool newline);

/** Short version of myDebugInfo.
  * fromWebServer prevents recursive calls when from WebServer
  */
void MyDbg(String info, bool fromWebServer = false, bool newline = true)
{
   myDebugInfo(info, fromWebServer, newline);
}


/** This function has to be overwritten to implement background delay calls. */
void myDelayLoop();

/** Replacement with background calls when we have to wait. Replace the delay function. */
void MyDelay(long millisDelay)
{
   long m = millis();

   while (millis() - m < millisDelay) {
      myDelayLoop();
      yield();
      delay(1);
   }
}


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
  * Trims the data string on the left and right side every occurrence of a char from chars.
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

/** Helper function to format a int with two digits (with '0' if needed */
String print2(int value) 
{
   if (value < 10) {
      return "0" + String(value);
   } else {
      return String(value);
   }
}

/** Helper function to format seconds to x days hours:minutes:seconds */
String formatInterval(long secs)
{
   int days    =  secs / 60 / 60 / 24;
   int hours   = (secs / 60 / 60) % 24;
   int minutes = (secs / 60) % 60;
   int seconds =  secs % 60;

   if (days <= 0) {
      return print2(hours) + ":" + print2(minutes) + ":" + print2(seconds); 
   } else {
      return String(days) + " " + print2(hours) + ":" + print2(minutes) + ":" + print2(seconds); 
   }
}

/** Helper function to scan a interval information '[days] hours:minutes:seconds' */
bool scanInterval(String interval, long &secs)
{
   int first = interval.indexOf(":");

   interval = Trim(interval, " ");
   if (first != -1) {
      String daysString;
      String hoursString;
      String minutesString;
      String secondsString;
      long   days    = 0;
      long   hours   = 0;
      long   minutes = 0;
      long   seconds = 0;

      int second = interval.indexOf(":", first + 1);

      if (second != -1) {
         int space = interval.indexOf(" ");

         if (space != -1 && space < first) {
            daysString  = interval.substring(0, space);
            hoursString = interval.substring(space + 1, first);
         } else {
            hoursString = interval.substring(0, first);
         }
         minutesString = interval.substring(first + 1, second);
         secondsString = interval.substring(second + 1);

         days    = atol(daysString.c_str());
         hours   = atol(hoursString.c_str());
         minutes = atol(minutesString.c_str());
         seconds = atol(secondsString.c_str());

         if (days    >= 0 && 
             hours   <= 0 && hours   <= 23 && 
             minutes >= 0 && minutes <= 59 && 
             seconds >= 0 && seconds <= 59) {
            secs = 0;
            secs += days    * 24 * 60 * 60;
            secs += hours   * 60 * 60;
            secs += minutes * 60;
            secs += seconds;
            return true;
         }
      }
   }
   return false;
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
