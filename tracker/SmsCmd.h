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
  * @file SmsCmd.h
  * 
  * Implementation of SMS interaction to the Modul.
  */

/**
  * SMS Controller class to manage receiving SMS commands.
  */
class MySmsCmd
{
protected:
   MyGsmGps  &myGsmGps;  //!< Reference to the gsm/gps instance.
   MyOptions &myOptions; //!< Reference to the options.
   MyData    &myData;    //!< Reference to the data.

protected:   
   void checkSms   ();   

   String getGoogleMapGpsUrl();

   void sendSms    (const String &message);
   void sendOk     (const SmsData &sms);

   bool readValues (String &value, const String message);
   bool readValues (long   &value, const String message);
   bool readValues (long   &value, long &value2, const String message);
   
   void cmdOn      (const SmsData &sms);
   void cmdOff     (const SmsData &sms);
   void cmdStatus  (const SmsData &sms);
   void cmdPsm     (const SmsData &sms);
   void cmdGps     (const SmsData &sms);
   void cmdSms     (const SmsData &sms);
   void cmdMqtt    (const SmsData &sms);
   void cmdPhone   (const SmsData &sms);
   void cmdDefault (const SmsData &sms);
      
public:
   MySmsCmd(MyGsmGps &gsmGps, MyOptions &options, MyData &data);

   bool begin();
   void handleClient();
};

/* ******************************************** */

/** Constructor */
MySmsCmd::MySmsCmd(MyGsmGps &gsmGps, MyOptions &options, MyData &data)
   : myGsmGps(gsmGps)
   , myOptions(options)
   , myData(data)
{
}

/** Log only the start of the sms controller */
bool MySmsCmd::begin()
{
   MyDbg("MySmsCmd::begin");
   return true;
}

/** Check the sms if the time from the options is elapsed. */
void MySmsCmd::handleClient()
{
   if (secondsElapsed(myData.rtcData.lastSmsCheckSec, myOptions.smsCheckIntervalSec)) {
      checkSms();
      if (myData.receivedCall) {
         SmsData sms;
         
         cmdStatus(sms);
         myData.receivedCall = false;
      }
   }
}

/** Returns the gps position as an google map url. */
String MySmsCmd::getGoogleMapGpsUrl()
{
   if (myData.gps.fixStatus) {
      return "https://maps.google.com/maps?q=" + myData.gps.latitudeString() + "," + myData.gps.longitudeString();   
   } else {
      if (myOptions.isGpsEnabled) {
         return "No Gps position.\n";
      } else {
         return "Gps not enabled.\n";
      }
   }
}

/** Checks for new sms and parse the commands from the message */
void MySmsCmd::checkSms()
{
   if (!myGsmGps.isGsmActive) {
      return;
   }
   
   SmsData sms;

   MyDbg("checkSMS");
   myData.deepSleepLocked = true;
   while (myGsmGps.getSMS(sms)) {
      String messageLower = sms.message;

      messageLower.toLowerCase();
      myGsmGps.deleteSMS(sms.index);

      MyDbg("SMS: " + sms.message + " ["+ sms.phoneNumber + "]");
      if (messageLower == "on") {
         cmdOn(sms);
      } else if (messageLower == "off") {
         cmdOff(sms);
      } else if (messageLower == "status") {
         cmdStatus(sms);
      } else if (messageLower == "psm") {
         cmdPsm(sms);
      } else if (messageLower == "gps") {
         cmdGps(sms);
      } else if (messageLower == "sms") {
         cmdSms(sms);
      } else if (messageLower == "mqtt") {
         cmdMqtt(sms);
      } else if (messageLower == "phone") {
         cmdPhone(sms);
      } else {
         cmdDefault(sms);
      }
   }
   myData.deepSleepLocked = false;
}

/** Helper function to send one sms */
void MySmsCmd::sendSms(const String &message)
{
   myGsmGps.sendSMS(myOptions.phoneNumber, message);
}

/** Send an OK sms */
void MySmsCmd::sendOk(const SmsData &sms)
{
   sendSms(sms.message + " -> OK");
}

/** Parse a sub string parameter from a sms message (xxx:sub) */
bool MySmsCmd::readValues(String &value, const String message)
{
   int idx = message.indexOf(':');

   if (idx != -1) {
      value = message.substring(idx + 1);
      value.toLowerCase();
      return true;
   }
   return false;
}

/** Parse a sub long parameter from a sms message (xxx:sub) */
bool MySmsCmd::readValues(long &value, const String message)
{
   int idx = message.indexOf(':');

   if (idx != -1) {
      value = atoi(message.substring(idx + 1).c_str());
      return true;
   }
   return false;
}

/** Parse two sub long parameter from a sms message (xxx:sub1:sub2) */
bool MySmsCmd::readValues(long &value1, long &value2, const String message)
{
   int first = message.indexOf(':');

   if (first != -1) {
      int second = message.indexOf(':', first + 1);

      if (second != -1) {
         value1 = atoi(message.substring(first + 1, second).c_str());
         value2 = atoi(message.substring(second + 1).c_str());
         return true;
      }
   }
   return false;
}

/** Command: switch on the modules */
void MySmsCmd::cmdOn(const SmsData &sms)
{
   myOptions.gsmPower     = true;
   myOptions.isGsmEnabled = true;
   myOptions.isGpsEnabled = true;
   sendOk(sms);
}

/** Command: switch off the modules */
void MySmsCmd::cmdOff(const SmsData &sms)
{
   myOptions.gsmPower = false;
   sendOk(sms);
}

/** Command: send status information via sms */
void MySmsCmd::cmdStatus(const SmsData &sms)
{
   String status;

   status += "Status: "       + myData.status              + '\n';
   status += "Voltage: "      + String(myData.voltage, 1)  + " V\n";
   status += "Temperature: "  + String(myData.temperature) + " C\n";
   status += "Humidity: "     + String(myData.humidity)    + " %\n";
   status += "Pressure: "     + String(myData.pressure)    + " hPa\n";
   if (!myData.gps.fixStatus) {
      if (myOptions.isGpsEnabled) {
         status += "No Gps positions.\n";
      } else {
         status += "Gps not enabled.\n";
      }
   } else {
      status += "Altitude: "   + myData.gps.altitudeString()   + " m\n";
      status += "Speed: "      + myData.gps.kmphString()       + " kmph\n";
      status += "Satellites: " + myData.gps.satellitesString() + '\n';
      status += getGoogleMapGpsUrl() + '\n';
   }
   sendSms(status);
}

/** Command: switch the power saving mode on or off. */
void MySmsCmd::cmdPsm(const SmsData &sms)
{
   if (sms.message.indexOf(":") == -1) {
      myOptions.isDeepSleepEnabled = true;
      sendOk(sms);
   } else {
      String off;

      if (readValues(off, sms.message) && off == "off") {
         myOptions.isDeepSleepEnabled = true;
         sendOk(sms);
      } else {
         cmdDefault(sms);
      }
   }
}

/** Command: send the gps position as an google map URL. */
void MySmsCmd::cmdGps(const SmsData &sms)
{
   if (sms.message.indexOf(":") == -1) {
      sendSms(getGoogleMapGpsUrl());
   } else {
      if (readValues(myOptions.gpsCheckIntervalSec, sms.message)) {
         sendOk(sms);
      } else {
         cmdDefault(sms);
      }
   }
}

/** Command: Set the sms checking time. */
void MySmsCmd::cmdSms(const SmsData &sms)
{
   if (readValues(myOptions.smsCheckIntervalSec, sms.message)) {
      sendOk(sms);
   } else {
      cmdDefault(sms);
   }
}

/** Command: Set the mqtt sending time checking time values. */
void MySmsCmd::cmdMqtt(const SmsData &sms)
{
   if (readValues(myOptions.mqttSendOnMoveEverySec, myOptions.mqttSendOnNonMoveEverySec, sms.message)) {
      sendOk(sms);
   } else {
      cmdDefault(sms);
   }
}

/** Command: Set the receiving phone number. */
void MySmsCmd::cmdPhone(const SmsData &sms)
{
   if (readValues(myOptions.phoneNumber, sms.message)) {
      sendOk(sms);
   } else {
      cmdDefault(sms);
   }
}

/** Default sms response if something is wrong */
void MySmsCmd::cmdDefault(const SmsData &sms)
{
   String info;

   info += "wrong command\n";
   info += "on\n";
   info += "off\n";
   info += "status\n";
   info += "psm[:off] - power saving mode\n";
   info += "gps[:15] - check every (sec)\n";
   info += "sms[:15] - check every (sec)\n";
   info += "mqtt[30:60] - (moving:standing (sec)\n";
   info += "phone:1234\n";
   sendSms(info);
}
