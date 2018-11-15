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
  * @file DeepSleep.h
  * 
  * DeepSleep functions.
  */


/**
  * Class to read/save a deepsleep counter and start the deepsleep mode
  * if the voltage is too low.
  */
class MyDeepSleep
{
protected:
   MyOptions &myOptions;           //!< Reference to the options
   MyData    &myData;              //!< Reference to the data
   
   uint32_t   awakeOffsetSec;      //!< awake time offset on reset.

public:
   MyDeepSleep(MyOptions &options, MyData &data);

   bool begin();
   
   bool haveToSleep();
   void sleep();
};

/* ******************************************** */

/** Constructor */
MyDeepSleep::MyDeepSleep(MyOptions &options, MyData &data)
   : myOptions(options)
   , myData(data)
   , awakeOffsetSec(0)
{
}

/**
  * Read the deepsleep counter from the RTC memory.
  * Use a simple random value variable to identify if the counter is still 
  * initialized
  */
bool MyDeepSleep::begin()
{
   MyDbg("MyDeepSleep::begin");
   
   MyData::RtcData rtcData;

   ESP.rtcUserMemoryRead(0, (uint32_t *) &rtcData, sizeof(MyData::RtcData));
   if (!rtcData.isValid()) {
      MyDbg("RtcData invalid");
   } else {
      MyDbg("RtcData read");
      myData.rtcData = rtcData;
      myData.rtcData.powerCheckCounter++;
   }

   if (myOptions.isDeepSleepEnabled) {
      if (myData.voltage < myOptions.powerSaveModeVoltage) {
         uint32_t checkTimeElapsed = myData.rtcData.powerCheckCounter * myOptions.powerCheckIntervalSec;

         // Check from time to time the power and return to deep sleep if the 
         // power is too low until the deep sleep time is over.
         MyDbg("CheckTime elapsed: " + String(checkTimeElapsed) + " sec");
         if (checkTimeElapsed < myOptions.deepSleepTimeSec) {
            sleep();
         }
      }
   }
   myData.rtcData.powerCheckCounter = 0;
   myData.secondsAwakeTime = millis() / 1000;
   MyDbg("DeepSleepCounter: " + String(myData.rtcData.deepSleepCounter));
   
   return true;
}

/** Check if the configured time has elapsed and the voltage is too low then go into deep sleep. */
bool MyDeepSleep::haveToSleep()
{
   if (myData.secondsAwakeTime == -1) {
      awakeOffsetSec = millis() / 1000;
   }
   myData.secondsAwakeTime   = millis() / 1000 - awakeOffsetSec;
   myData.secondsToDeepSleep = -1;
   if (myOptions.isDeepSleepEnabled && myData.voltage < myOptions.powerSaveModeVoltage) {
      myData.secondsToDeepSleep = myOptions.wakeTimeSec - myData.secondsAwakeTime;
   }

   return (myOptions.isDeepSleepEnabled && 
           myData.voltage          < myOptions.powerSaveModeVoltage && 
           myData.secondsAwakeTime > myOptions.wakeTimeSec);
}

/** Entering the DeepSleep mode. Be sure we have connected the RST pin to the D0 pin for wakup. */
void MyDeepSleep::sleep()
{
   MyDbg("Entering DeepSleep: " + String(myOptions.powerCheckIntervalSec) + "Sec");

   myData.rtcData.deepSleepCounter++;
   myData.rtcData.setCRC();
   ESP.rtcUserMemoryWrite(0, (uint32_t *) &myData.rtcData, sizeof(MyData::RtcData));
   ESP.deepSleep(myOptions.powerCheckIntervalSec * 1000000);
}
