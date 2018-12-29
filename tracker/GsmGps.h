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
  * @file GsmGps.h
  *
  * Class to communicate between the esp8266 and the sim808 module.
  */


#define  TINY_GSM_YIELD() { MyDelay(1); } //!< Overwrite the yield macro with our own delay function.
#include "Sim808.h"
#include "Serial.h"

/**
  * SIM808 Communication class to handle gprs and gps activities.
  */
class MyGsmGps
{
protected:
   long          lastGsmChecSec;   //!< Check intervall for signal and battery quality.
   long          lastGpsCheckSec;  //!< GPS Check intervall
   long          startGpsCheck;    //!< Timstamp of first getGps try

public:
   MySerial      gsmSerial;        //!< Serial interface to the sim808 modul.
   MyGsmSim808   gsmSim808;        //!< SIM808 interface class 
   TinyGsmClient gsmClient;        //!< Gsm client interface
   
   bool          isGsmActive;      //!< Is the sim808 modul started?
   bool          isGpsActive;      //!< Is the gs part of the sim808 activated?

   MyOptions    &myOptions;        //!< Reference to the options.
   MyData       &myData;           //!< Reference to the data.

protected:
   void enableGps(bool enable);
   bool getGps();
   bool sleepMode2();

public:
   MyGsmGps(MyOptions &options, MyData &data, short pinRx, short pinTx);

   bool begin();
   void handleClient();
   bool stop();
   bool waitingForGps();

   bool sendAT(String cmd);

   bool getSMS(SmsData &sms);
   bool sendSMS(String phoneNumber, String message);
   bool deleteSMS(long index);
};

/* ******************************************** */

/** Constructor */
MyGsmGps::MyGsmGps(MyOptions &options, MyData &data, short pinRx, short pinTx)
   : gsmSerial(data.logInfos, options.isDebugActive, pinRx, pinTx)
   , gsmSim808(gsmSerial)
   , gsmClient(gsmSim808)
   , isGsmActive(false)
   , isGpsActive(false)
   , myOptions(options)
   , myData(data)
   , lastGsmChecSec(0)
   , lastGpsCheckSec(0)
   , startGpsCheck(0)
{
   gsmSerial.begin(9600);
}

/** Initialized the sim808 modul and start optionally the gsm and/ or gps part. */
bool MyGsmGps::begin()
{
   if (!myOptions.gsmPower) {
      MyDbg(F("sim808 has no power!"));
      return false;
   }

   if (!isGsmActive) {
      MyDbg(F("MyGsmGps::begin"));
      myData.status = F("Sim808 Initializing...");
      MyDbg(myData.status);
      for (int i = 0; !gsmSim808.restart() && i <= 5; i++) {
         if (!myOptions.gsmPower) {
            MyDbg(F("Sim808 Initializing ... canceled"));
            return false;
         }
         if (i == 5) { // not working!
            myData.status = F("Sim808 restart failed");
            MyDbg(myData.status);
            return false;
         }
         MyDbg(F("."), false, false);
         MyDelay(500);
      }
      myData.status = F("Sim808 connected");
      MyDbg(myData.status);

      gsmSim808.setBaud(9600);

      myData.status = F("Sim808 Waiting for network...");
      MyDbg(myData.status);
      for (int i = 0; !gsmSim808.waitForNetwork() && i <= 5; i++) {
         if (!myOptions.gsmPower) {
            MyDbg(F("Sim808 Waiting for network... canceled"));
            return false;
         }
         if (i == 5) { // not working!
            myData.status = F("Sim808 network failed");
            MyDbg(myData.status);
            return false;
         }
         MyDbg(F("."), false, false);
         MyDelay(500);
      }
      if (!gsmSim808.isNetworkConnected()) {
         myData.status = F("Sim808 network failed");
         MyDbg(myData.status);
      } else {
         myData.status = F("Sim808 network connected");
         MyDbg(myData.status);

         MyDbg((String) F("GPRS: ") + myOptions.gprsAP);
         if (!gsmSim808.gprsConnect(myOptions.gprsAP.c_str(), "", "")) {
            myData.status = F("Sim808 gprs connection failed!");
            MyDbg(myData.status);
            return false;
         }
         myData.status = F("Sim808 gsm connected");
         MyDbg(myData.status);

         myData.modemInfo = gsmSim808.getModemInfo();
         MyDbg((String) F("Modem info: ") + myData.modemInfo);

         myData.modemIP = gsmSim808.getLocalIP();
         MyDbg((String) F("Modem IP: ") + myData.modemIP);

         myData.imei = gsmSim808.getIMEI();
         MyDbg((String) F("sim808: ") + myData.modemInfo);

         myData.cop = gsmSim808.getOperator();
         MyDbg((String) F("cop: ") + myData.modemInfo);
      }
      isGsmActive = true;
   }
   
   if (isGsmActive && myOptions.isGpsEnabled && !isGpsActive) {
      enableGps(true);
      isGpsActive = true;
   }

   return true;
}

/** Checks the gps from time to time if enabled. */
void MyGsmGps::handleClient()
{
   if (!isGsmActive) {
      return;
   }

   if (secondsElapsedAndUpdate(lastGsmChecSec, 60000)) { // Check every minute
      myData.signalQuality = String(gsmSim808.getSignalQuality());
      myData.batteryLevel  = String(gsmSim808.getBattPercent());
      myData.batteryVolt   = String(gsmSim808.getBattVoltage() / 1000.0F, 2);

      MyDbg((String) F("(sim808) signalQuality: ") + myData.signalQuality);
      MyDbg((String) F("(sim808) batteryLevel: ")  + myData.batteryLevel);
      MyDbg((String) F("(sim808) batteryVolt: ")   + myData.batteryVolt);
   }

   if (myOptions.isGpsEnabled) {
      if (secondsElapsedAndUpdate(lastGpsCheckSec, 10)) { // Wait 10 sec between retries
         if (secondsElapsed(myData.rtcData.lastGpsReadSec, myOptions.gpsCheckIntervalSec)) {
            if (!isGpsActive) {
               enableGps(true);
            }
            MyDbg(F("getGPS"));
            if (startGpsCheck == 0) {
               startGpsCheck = secondsSincePowerOn();
            }
            if (getGps()) {
               MyDbg(F(" -> ok"));
               startGpsCheck = 0;
               myData.rtcData.lastGpsReadSec = secondsSincePowerOn();
            } else {
               long waitForGpsTime = secondsSincePowerOn() - startGpsCheck;

               // Ignore gps if we cannot get a position in X minutes.
               if (waitForGpsTime > myOptions.gpsTimeoutSec) {
                  MyDbg(F(" -> gps timeout!"));
                  startGpsCheck = 0;
                  myData.gps.hasTimeout = true;
                  myData.rtcData.lastGpsReadSec = secondsSincePowerOn();
               } else {
                  if (myOptions.gpsTimeoutSec - waitForGpsTime > 0) {
                     MyDbg((String) F(" -> no gps fix (timeout in ") + String(myOptions.gpsTimeoutSec - waitForGpsTime) + F(" seconds!)"));
                  }
               }
            }
         }
      }
   }
}

/** Stops the sim808 modul and go to deep sleep mode. */
bool MyGsmGps::stop()
{
   bool ret = true;
   
   MyDbg(F("gprs gps stopping"));
   enableGps(false);
   if (gsmSim808.isGprsConnected()) {
      ret = gsmSim808.gprsDisconnect();
   }  
   if (ret) {
      isGpsActive = false;
      isGsmActive = false;
      MyDbg(F("gprs gps stopped"));
      myData.status = F("Sim808 stopped!");
      sleepMode2();
   }
   return ret;
}

/** Is the gps enabled but we don't have a valid gps position. */
bool MyGsmGps::waitingForGps()
{
   return isGsmActive && myOptions.isGpsEnabled && !myData.gps.fixStatus && !myData.gps.hasTimeout;
}

/** Send one AT command to the sim modul and log the result for the console window. */
bool MyGsmGps::sendAT(String cmd)
{
   if (!isGsmActive) {
      MyDbg(F("sim808 not active!"));
      return false;
   }

   String response;

   gsmSerial.print(cmd);
   gsmSerial.print(F("\r\n"));
   gsmSim808.waitResponse(1000, response);
   MyDbg(response);
   return true;
}

/** Entering the power save mode of the sim808 modul. */
bool MyGsmGps::sleepMode2()
{
   MyDbg(F("Entering gsm sleep mode 2"));
   return sendAT(GF("+CSCLK=2"));
}

/** Read one sms if available. */
bool MyGsmGps::getSMS(SmsData &sms)
{
   if (!isGsmActive) {
      MyDbg(F("gsm not active!"));
      return false;
   }

   MyDbg(F("getSMS"));
   return gsmSim808.getSMS(sms);
}

/** Send one sms to a specific phone number via gsm. */
bool MyGsmGps::sendSMS(String phoneNumber, String message)
{
   if (!isGsmActive) {
      MyDbg(F("gsm not active!"));
      return false;
   }

   MyDbg((String) F("sendSMS: ") + message);
   return gsmSim808.sendSMS(phoneNumber, message);
}

/** Delete one specific sms from the sim card. */
bool MyGsmGps::deleteSMS(long index)
{
   if (!isGsmActive) {
      MyDbg(F("gsm not active!"));
      return false;
   }

   MyDbg((String) F("deleteSMS: ") + String(index));
   return gsmSim808.deleteSMS(index);
}

/** Switch on the gps part of the sim808 modul. */
void MyGsmGps::enableGps(bool enable)
{
   if (!isGsmActive) {
      MyDbg(F("sim808 not active!"));
      return;
   }

   if (enable) {
      gsmSim808.enableGPS();
      myData.status = F("Sim808 gps enabled!");
      MyDbg(myData.status);
      isGpsActive = true;
   } else {
      gsmSim808.disableGPS();
      myData.status = F("Sim808 gps disabled!");
      MyDbg(myData.status);
      isGpsActive = false;
   }
}

/** Read one gps position with the sim808 modul and save the values in the global data. */
bool MyGsmGps::getGps()
{
   if (!isGsmActive) {
      MyDbg(F("sim808 not active!"));
      return false;
   }

   bool ret = false;

   if (isGpsActive) {
      if (gsmSim808.getGPS(myData.gps)) {
         myData.lastGpsUpdateSec = secondsSincePowerOn();

         MyDbg((String) F("(gps) longitude: ")  + myData.gps.longitudeString());
         MyDbg((String) F("(gps) latitude: ")   + myData.gps.latitudeString());
         MyDbg((String) F("(gps) altitude: ")   + myData.gps.altitudeString());
         MyDbg((String) F("(gps) kmph: ")       + myData.gps.kmphString());
         MyDbg((String) F("(gps) satellites: ") + myData.gps.satellitesString());
         MyDbg((String) F("(gps) course: ")     + myData.gps.courseString());
         MyDbg((String) F("(gps) gpsDate: ")    + myData.gps.dateString());
         MyDbg((String) F("(gps) gpsTime: ")    + myData.gps.timeString());

         if (myData.rtcData.lastLocation.latitude() != 0) {
            myData.movingDistance = myData.gps.location.distanceTo(myData.rtcData.lastLocation);
            myData.isMoving       = myData.movingDistance > myOptions.minMovingDistance;
         }
         myData.rtcData.lastLocation = myData.gps.location;
         ret = true;
      }
   }
   return ret;
}
