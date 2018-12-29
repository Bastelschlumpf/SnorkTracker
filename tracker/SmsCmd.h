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
   MyDbg(F("MySmsCmd::begin"));
   return true;
}

/** Check the sms if the time from the options is elapsed. */
void MySmsCmd::handleClient()
{
   if (secondsElapsedAndUpdate(myData.rtcData.lastSmsCheckSec, myOptions.smsCheckIntervalSec)) {
      checkSms();
   }
}

/** Returns the gps position as an google map url. */
String MySmsCmd::getGoogleMapGpsUrl()
{
   if (myData.gps.fixStatus) {
      return (String) F("https://maps.google.com/maps?q=") + myData.gps.latitudeString() + F(",") + myData.gps.longitudeString();
   } else {
      if (myOptions.isGpsEnabled) {
         return F("No Gps position.\n");
      } else {
         return F("Gps not enabled.\n");
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

   MyDbg(F("checkSMS"));
   while (myGsmGps.getSMS(sms)) {
      String messageLower = sms.message;

      messageLower.toLowerCase();
      myGsmGps.deleteSMS(sms.index);

      MyDbg((String) F("SMS: ") + sms.message + F(" [") + sms.phoneNumber + F("]"));
      if (messageLower.indexOf(F("on")) == 0) {
         cmdOn(sms);
      } else if (messageLower.indexOf(F("off")) == 0) {
         cmdOff(sms);
      } else if (messageLower.indexOf(F("status")) == 0) {
         cmdStatus(sms);
      } else if (messageLower.indexOf(F("psm")) == 0) {
         cmdPsm(sms);
      } else if (messageLower.indexOf(F("gps")) == 0) {
         cmdGps(sms);
      } else if (messageLower.indexOf(F("sms")) == 0) {
         cmdSms(sms);
      } else if (messageLower.indexOf(F("mqtt")) == 0) {
         cmdMqtt(sms);
      } else if (messageLower.indexOf(F("phone")) == 0) {
         cmdPhone(sms);
      } else {
         cmdDefault(sms);
      }
   }
}

/** Helper function to send one sms */
void MySmsCmd::sendSms(const String &message)
{
   myGsmGps.sendSMS(myOptions.phoneNumber, message);
}

/** Send an OK sms */
void MySmsCmd::sendOk(const SmsData &sms)
{
   sendSms(sms.message + F(" -> OK"));
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
   myOptions.isGpsEnabled = true;
   myOptions.save();
   sendOk(sms);
}

/** Command: switch off the modules */
void MySmsCmd::cmdOff(const SmsData &sms)
{
   myOptions.gsmPower = false;
   myOptions.save();
   sendOk(sms);
}

/** Command: send status information via sms */
void MySmsCmd::cmdStatus(const SmsData &sms)
{
   String status;

   status += (String) F("Status: ")      + myData.status              + '\n';
   status += (String) F("Voltage: ")     + String(myData.voltage, 1)  + F(" V\n");
   status += (String) F("Temperature: ") + String(myData.temperature) + F(" C\n");
   status += (String) F("Humidity: ")    + String(myData.humidity)    + F(" %\n");
   status += (String) F("Pressure: ")    + String(myData.pressure)    + F(" hPa\n");
   if (!myData.gps.fixStatus) {
      if (myOptions.isGpsEnabled) {
         status += F("No Gps positions.");
      } else {
         status += F("Gps not enabled.");
      }
   } else {
      status += (String) F("Altitude: ")   + myData.gps.altitudeString()   + F(" m\n");
      status += (String) F("Speed: ")      + myData.gps.kmphString()       + F(" kmph\n");
      status += (String) F("Satellites: ") + myData.gps.satellitesString() + '\n';
      status += getGoogleMapGpsUrl();
   }
   sendSms(status);
}

/** Command: switch the power saving mode on or off. */
void MySmsCmd::cmdPsm(const SmsData &sms)
{
   if (sms.message.indexOf(F(":")) == -1) {
      myOptions.isDeepSleepEnabled = true;
      myOptions.save();
      sendOk(sms);
   } else {
      String off;

      if (readValues(off, sms.message) && off == F("off")) {
         myOptions.isDeepSleepEnabled = true;
         myOptions.save();
         sendOk(sms);
      } else {
         MyDbg((String) F("psm:[") + off + F("]"));
         cmdDefault(sms);
      }
   }
}

/** Command: send the gps position as an google map URL. */
void MySmsCmd::cmdGps(const SmsData &sms)
{
   if (sms.message.indexOf(F(":")) == -1) {
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

   info += F("wrong command\n");
   info += F("on\n");
   info += F("off\n");
   info += F("status\n");
   info += F("psm[:off] - power saving mode\n");
   info += F("gps[:15] - check every (sec)\n");
   info += F("sms[:15] - check every (sec)\n");
   info += F("mqtt[30:60] - (moving:standing (sec)\n");
   info += F("phone:1234\n");
   sendSms(info);
}
