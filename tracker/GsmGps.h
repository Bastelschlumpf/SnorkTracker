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
public:
   MySerial         gsmSerial;         //!< Serial interface to the sim808 modul.
   MyGsmSim808      gsmSim808;         //!< SIM808 interface class 
   TinyGsmClient    gsmClient;         //!< Gsm client interface
   
   bool             isSimActive;      //!< Is the sim808 modul started?
   bool             isGsmActive;      //!< Is the gsm part of the sim808 activated?
   bool             isGpsActive;      //!< Is the gs part of the sim808 activated?

   MyOptions       &myOptions;        //!< Reference to the options.
   MyData          &myData;           //!< Reference to the data.

protected:
   void enableGps(bool enable);
   bool getGps();
   bool sleepMode2();

public:
   MyGsmGps(MyOptions &options, MyData &data, short pinRx, short pinTx);

   bool begin();
   void handleClient();
   bool stop();

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
   , isSimActive(false)
   , isGsmActive(false)
   , isGpsActive(false)
   , myOptions(options)
   , myData(data)
{
   gsmSerial.begin(9600);
}

/** Initialized the sim808 modul and start optionally the gsm and/ or gps part. */
bool MyGsmGps::begin()
{
   if (!myOptions.gsmPower) {
      MyDbg("sim808 has no power!");
      return false;
   }

   if (!isSimActive) {
      MyDbg("MyGsmGps::begin");
      myData.status = "Sim808 Initializing...";
      MyDbg(myData.status);
      for (int i = 0; !gsmSim808.restart() && i <= 5; i++) {
         if (!myOptions.gsmPower) {
            MyDbg("Sim808 Initializing ... canceled");
            return false;
         }
         if (i == 5) { // not working!
            myData.status = "Sim808 restart failed";
            MyDbg(myData.status);
            return false;
         }
         MyDbg(".", false, false);
         MyDelay(500);
      }
      myData.status = "Sim808 connected";
      MyDbg(myData.status);

      gsmSim808.setBaud(9600);
      isSimActive = true;
   }
   
   if (isSimActive && myOptions.isGpsEnabled && !isGpsActive) {
      enableGps(true);
      isGpsActive = true;
   }

   if (isSimActive && myOptions.isGsmEnabled && !isGsmActive) {
      myData.status = "Sim808 Waiting for network...";
      MyDbg(myData.status);
      for (int i = 0; !gsmSim808.waitForNetwork() && i <= 5; i++) {
         if (!myOptions.gsmPower) {
            MyDbg("Sim808 Waiting for network... canceled");
            return false;
         }
         if (i == 5) { // not working!
            myData.status = "Sim808 network failed";
            MyDbg(myData.status);
            return false;
         }
         MyDbg(".", false, false);
         MyDelay(500);
      }
      if (!gsmSim808.isNetworkConnected()) {
         myData.status = "Sim808 network failed";
         MyDbg(myData.status);
      }

      MyDbg((String) "GPRS: " + myOptions.gprsAP);
      if (!gsmSim808.gprsConnect(myOptions.gprsAP.c_str(), "", "")) {
         myData.status = "Sim808 gprs connection failed!";
         MyDbg(myData.status);
         while (true);
      }
      myData.status = "Sim808 gsm connected";
      MyDbg(myData.status);

      myData.modemInfo = gsmSim808.getModemInfo();
      MyDbg("Modem info: " + myData.modemInfo);

      myData.modemIP = gsmSim808.getLocalIP();
      MyDbg("Modem IP: " + myData.modemIP);

      myData.imei = gsmSim808.getIMEI();
      MyDbg("sim808: " + myData.modemInfo);

      myData.cop = gsmSim808.getOperator();
      MyDbg("cop: " + myData.modemInfo);

      isGsmActive = true;
   }
   return true;
}

/** Checks the gps from time to time if enabled. */
void MyGsmGps::handleClient()
{
   if (!isSimActive) {
      return;
   }

   if (secondsElapsed(myData.rtcData.lastGpsReadSec, myOptions.gpsCheckIntervalSec)) {
      if (myOptions.isGpsEnabled && !isGpsActive) {
         enableGps(true);
      }
      getGps();
   }
}

/** Stops the sim808 modul and go to deep sleep mode. */
bool MyGsmGps::stop()
{
   bool ret = true;
   
   MyDbg("gprs gps stopping");
   enableGps(false);
   if (gsmSim808.isGprsConnected()) {
      ret = gsmSim808.gprsDisconnect();
   }  
   if (ret) {
      isGsmActive = false;
      isGpsActive = false;
      isSimActive = false;
      MyDbg("gprs gps stopped");
      myData.status = "Sim808 stopped!";
      sleepMode2();
   }
   return ret;
}

/** Send one AT command to the sim modul and log the result for the console window. */
bool MyGsmGps::sendAT(String cmd)
{
   if (!isSimActive) {
      MyDbg("sim808 not active!");
      return false;
   }

   String response;

   gsmSerial.print(cmd);
   gsmSerial.print("\r\n");
   gsmSim808.waitResponse(1000, response);
   MyDbg(response);
   return true;
}

/** Entering the power save mode of the sim808 modul. */
bool MyGsmGps::sleepMode2()
{
   MyDbg("Entering gsm sleep mode 2");
   return sendAT(GF("+CSCLK=2"));
}

/** Read one sms if available. */
bool MyGsmGps::getSMS(SmsData &sms)
{
   if (!isGsmActive) {
      MyDbg("gsm not active!");
      return false;
   }

   MyDbg("getSMS");
   return gsmSim808.getSMS(sms);
}

/** Send one sms to a specific phone number via gsm. */
bool MyGsmGps::sendSMS(String phoneNumber, String message)
{
   if (!isGsmActive) {
      MyDbg("gsm not active!");
      return false;
   }

   MyDbg("sendSMS: " + message);
   return gsmSim808.sendSMS(phoneNumber, message);
}

/** Delete one specific sms from the sim card. */
bool MyGsmGps::deleteSMS(long index)
{
   if (!isGsmActive) {
      MyDbg("gsm not active!");
      return false;
   }

   MyDbg("deleteSMS: " + String(index));
   return gsmSim808.deleteSMS(index);
}

/** Switch on the gps part of the sim808 modul. */
void MyGsmGps::enableGps(bool enable)
{
   if (!isSimActive) {
      MyDbg("sim808 not active!");
      return;
   }

   if (enable) {
      gsmSim808.enableGPS();
      myData.status = "Sim808 gps enabled!";
      MyDbg(myData.status);
      isGpsActive = true;
   } else {
      gsmSim808.disableGPS();
      myData.status = "Sim808 gps disabled!";
      MyDbg(myData.status);
      isGpsActive = false;
   }
}

/** Read one gps position with the sim808 modul and save the values in the global data. */
bool MyGsmGps::getGps()
{
   if (!isSimActive) {
      MyDbg("sim808 not active!");
      return false;
   }

   bool ret = false;

   if (isGpsActive) {
      MyDbg("getGPS");
      if (gsmSim808.getGPS(myData.gps)) {
         myData.lastGpsUpdateSec = secondsSincePowerOn();

         myData.signalQuality    = String(gsmSim808.getSignalQuality());
         myData.batteryLevel     = String(gsmSim808.getBattPercent());
         myData.batteryVolt      = String(gsmSim808.getBattVoltage() / 1000.0F, 6);
            
         MyDbg("(sim808) signalQuality: " + myData.signalQuality);
         MyDbg("(sim808) batteryLevel: "  + myData.batteryLevel);
         MyDbg("(sim808) batteryVolt: "   + myData.batteryVolt);
         MyDbg("(gps) longitude: "        + myData.gps.longitudeString());
         MyDbg("(gps) latitude: "         + myData.gps.latitudeString());
         MyDbg("(gps) altitude: "         + myData.gps.altitudeString());
         MyDbg("(gps) kmph: "             + myData.gps.kmphString());
         MyDbg("(gps) satellites: "       + myData.gps.satellitesString());
         MyDbg("(gps) course: "           + myData.gps.courseString());
         MyDbg("(gps) gpsDate: "          + myData.gps.dateString());
         MyDbg("(gps) gpsTime: "          + myData.gps.timeString());
    
         if (myData.rtcData.lastLocation.latitude() != 0) {
            myData.movingDistance       = myData.gps.location.distanceTo(myData.rtcData.lastLocation);
            myData.isMoving             = myData.movingDistance > myOptions.minMovingDistance;
            myData.rtcData.lastLocation = myData.gps.location;
            ret = true;
         }
      }
   }
   return ret;
}
