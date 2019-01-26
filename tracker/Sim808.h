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
  * @file Sim808.h
  *
  * Extension class of the TinyGsmSim808 base class.
  */


#define TINY_GSM_MODEM_SIM808 //!< Defines the modul as a SIM808 type for the TinyGsmClient library 
#define TINY_GSM_DEBUG Serial //!< ???

#include <TinyGsmClient.h>

/** 
  * Helper class for storing one SMS data. 
  */
class SmsData
{
public:
   long   index;            //!< Sms index on sim card.
   String status;           //!< Sms status like read or not.
   String phoneNumber;      //!< Sms sender number.
   String referenceNumber;  //!< ???
   String dateTime;         //!< DateTime of the sms.
   String message;          //!< Sms content.
};

/**
  * Extension class of the TinyGsmSim808 base class.
  * Implements the special gps call and some sms functions.
  */
class MyGsmSim808 : public TinyGsmSim808
{
public:
   MyGsmSim808(Stream &stream);

   bool getGps    (MyGps &gps);
   bool getGsmGps (MyGps &gps);
   bool getSMS    (SmsData &sms);
   bool deleteSMS (long index);
};

/* ******************************************** */

/** Constructor */
MyGsmSim808::MyGsmSim808(Stream &stream)
   : TinyGsmSim808(stream)
{
}

/** Read and parse a gps information from the sim808 modul in the own MyGps data class */
bool MyGsmSim808::getGps(MyGps &gps)
{
   gps.clear();

   sendAT(GF("+CGNSINF"));
   if (waitResponse(GF(GSM_NL "+CGNSINF:")) != 1) {
      return false;
   }

   gps.setRunStatus        (stream.readStringUntil(','));
   gps.setFixStatus        (stream.readStringUntil(','));
   gps.setDateTime         (stream.readStringUntil(','));
   gps.setLatitude         (stream.readStringUntil(','));
   gps.setLongitude        (stream.readStringUntil(','));
   gps.setAltitude         (stream.readStringUntil(',')); 
   gps.setSpeed            (stream.readStringUntil(','));  
   gps.setCourse           (stream.readStringUntil(','));  
   gps.setFixMode          (stream.readStringUntil(','));   
   /* reserved */          (stream.readStringUntil(','));
   gps.setHdop             (stream.readStringUntil(','));  
   gps.setPdop             (stream.readStringUntil(','));  
   gps.setVdop             (stream.readStringUntil(',')); 
   /* reserved */          (stream.readStringUntil(','));
   gps.setSatellitesInView (stream.readStringUntil(','));
   gps.setSatellitesUsed   (stream.readStringUntil(','));  
   stream.readStringUntil('\n');
   waitResponse();
   
   return gps.fixStatus;
}

/** Read and parse a gsm-gps information from the sim808 modul in the own MyGps data class 
  * Sample: AT+CIPGSMLOC=1,1
  *         +CIPGSMLOC: 0,23.7798,61.496052,2019/01/26,08:21:47
  */
bool MyGsmSim808::getGsmGps(MyGps &gps)
{
   gps.clear();

   sendAT(GF("+CIPGSMLOC=1,1"));
   if (waitResponse(10000L, GF(GSM_NL "+CIPGSMLOC:")) != 1) {
      return false;
   }

   String locationCode = stream.readStringUntil(',');
   String longitude    = stream.readStringUntil(',');
   String latitude     = stream.readStringUntil(',');
   String gsmDate      = stream.readStringUntil(',');
   String gsmTime      = stream.readStringUntil(',');
   stream.readStringUntil('\n');
   waitResponse();

   locationCode.trim();
   if (locationCode == "0") {
      String dateTime = gsmDate + gsmTime;

      dateTime.replace("/", "");
      dateTime.replace(":", "");
      dateTime += ".000";
      gps.setLatitude(longitude);
      gps.setLongitude(latitude);
      gps.setDateTime(dateTime);
      gps.fixStatus = true;
   }

   return gps.fixStatus;
}

/** Read one SMS from the sim card into the own SmsData class. */
bool MyGsmSim808::getSMS(SmsData &sms)
{
   // PDU Mode (Hex)
   sendAT(GF("+CMGF=1")); 
   if (waitResponse() != 1) {
      return false;
   }
   
   // Read unread sms
   sendAT(GF("+CMGL=\"REC UNREAD\""));
   int status = waitResponse(GFP(GSM_OK), GF("+CMGL:"));

   if (status == 2) {
      sms.index           = atoi(stream.readStringUntil(',').c_str());
      sms.status          = stream.readStringUntil(',');
      sms.phoneNumber     = stream.readStringUntil(',');
      sms.referenceNumber = stream.readStringUntil(',');
      sms.dateTime        = stream.readStringUntil('\n');
      sms.message         = stream.readStringUntil('\n');
      sms.message         = Trim(sms.message, F("\r\n"));
      waitResponse(); 
      return true;
   }
       
   return false;
}

/** Delete a specific sms from the sim card. */
bool MyGsmSim808::deleteSMS(long index)
{
   sendAT(GF("+CMGD=") + String(index));
   if (waitResponse() != 1) {
      return false;
   }

   return true; 
}
