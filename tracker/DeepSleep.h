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

#define NO_DEEP_SLEEP_STARTUP_TIME 120 //!< No deep sleep for the first two minute.


/**
  * Class to read/save a deepsleep counter and start the deepsleep mode
  * if the voltage is too low.
  */
class MyDeepSleep
{
protected:
   MyOptions &myOptions;     //!< Reference to the options
   MyData    &myData;        //!< Reference to the data
   
public:
   MyDeepSleep(MyOptions &options, MyData &data);

   bool begin();
   
   bool haveToSleep();
   void sleep(bool start = true);
};

/* ******************************************** */

/** Constructor */
MyDeepSleep::MyDeepSleep(MyOptions &options, MyData &data)
   : myOptions(options)
   , myData(data)
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
   }

   if (myOptions.isDeepSleepEnabled && secondsSincePowerOn() > NO_DEEP_SLEEP_STARTUP_TIME) {
      if (myData.voltage < myOptions.powerSaveModeVoltage) {
         long checkTimeElapsed = secondsSincePowerOn() - myData.rtcData.deepSleepStartSec;

         // Check from time to time the power and return to deep sleep if the 
         // power is too low until the deep sleep time is over.
         MyDbg("CheckTime elapsed: " + String(checkTimeElapsed) + " sec");
         if (checkTimeElapsed < myOptions.deepSleepTimeSec) {
            sleep(false);
         }
         MyDbg("Awake");
      }
   }
   return true;
}

/** Check if the configured time has elapsed and the voltage is too low then go into deep sleep. */
bool MyDeepSleep::haveToSleep()
{
   long activeTimeSec = millis() / 1000 - myData.awakeTimeOffsetSec;

   myData.secondsToDeepSleep = -1;
   if (myOptions.isDeepSleepEnabled && myData.voltage < myOptions.powerSaveModeVoltage) {
      myData.secondsToDeepSleep = myOptions.activeTimeSec - activeTimeSec;
   }

   return (myOptions.isDeepSleepEnabled && 
           secondsSincePowerOn() > NO_DEEP_SLEEP_STARTUP_TIME &&
           myData.voltage        <  myOptions.powerSaveModeVoltage && 
           activeTimeSec         >= myOptions.activeTimeSec);
}

/** Entering the DeepSleep mode. Be sure we have connected the RST pin to the D0 pin for wakeup. */
void MyDeepSleep::sleep(bool start /* = true */)
{
   MyDbg("Entering DeepSleep for: " + String(myOptions.powerCheckIntervalSec) + "Sec");

   if (start) {
      myData.rtcData.deepSleepStartSec = secondsSincePowerOn();
   }
   myData.rtcData.aktiveTimeSec    += millis() / 1000;
   myData.rtcData.deepSleepTimeSec += myOptions.powerCheckIntervalSec;
   myData.rtcData.setCRC();
   ESP.rtcUserMemoryWrite(0, (uint32_t *) &myData.rtcData, sizeof(MyData::RtcData));
   ESP.deepSleep(myOptions.powerCheckIntervalSec * 1000000);
}
