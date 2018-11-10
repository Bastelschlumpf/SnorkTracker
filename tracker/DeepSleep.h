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

   long      wakeTimeStartSec;     //!< Second counter since wakeup
   uint32_t  deepSleepCounter;     //!< Counter of the deepsleep cycles
   uint32_t  deepSleepCounterInit; //!< Check if the counter was initialized

public:
   MyDeepSleep(MyOptions &options, MyData &data);

   bool begin();

   bool haveToSleep();
   void sleep();
};

/* ******************************************** */

#define RTC_CRC_VALUE 210665 //!< Fantasy value for checking if the counter is initialized (instead of a crc).

/** Constructor */
MyDeepSleep::MyDeepSleep(MyOptions &options, MyData &data)
   : myOptions(options)
   , myData(data)
   , wakeTimeStartSec(0)
   , deepSleepCounter(0)
   , deepSleepCounterInit(0)
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
   
   ESP.rtcUserMemoryRead(sizeof(uint32_t), &deepSleepCounterInit, sizeof(uint32_t));
   if (deepSleepCounterInit == RTC_CRC_VALUE) {
      ESP.rtcUserMemoryRead(0, &deepSleepCounter, sizeof(uint32_t));
      deepSleepCounter++;
   } else {
      deepSleepCounterInit = RTC_CRC_VALUE;
      ESP.rtcUserMemoryWrite(sizeof(uint32_t), &deepSleepCounterInit, sizeof(uint32_t));
   }
   ESP.rtcUserMemoryWrite(0, &deepSleepCounter, sizeof(uint32_t));

   if (myOptions.isDeepSleepEnabled) {
      if (myData.voltage < myOptions.powerSaveModeVoltage) {
         MyDbg("DepSleepCounter: " + String(deepSleepCounter));
         if (deepSleepCounter * myOptions.powerCheckIntervalSec < myOptions.deepSleepTimeSec) {
            sleep();
         }
      }
      deepSleepCounter = 0;
      ESP.rtcUserMemoryWrite(0, &deepSleepCounter, sizeof(uint32_t));
   }
   
   wakeTimeStartSec = millis() / 1000;
   
   return true;
}

/** Check if the configured time has elapsed and the voltage is too low then go into deep sleep. */
bool MyDeepSleep::haveToSleep()
{
   long wakeTimeSec = millis() / 1000 - wakeTimeStartSec;

   myData.secondsToDeepSleep = -1;
   if (myOptions.isDeepSleepEnabled && myData.voltage < myOptions.powerSaveModeVoltage) {
      myData.secondsToDeepSleep = myOptions.wakeTimeSec - wakeTimeSec;
   }

   return (myOptions.isDeepSleepEnabled && 
           wakeTimeSec    > myOptions.wakeTimeSec &&
           myData.voltage < myOptions.powerSaveModeVoltage);
}

/** Entering the DeepSleep mode. Be sure we have connected the RST pin to the D0 pin for wakup. */
void MyDeepSleep::sleep()
{
   MyDbg("Entering DeepSleep: " + String(myOptions.powerCheckIntervalSec) + "Sec");
   ESP.deepSleep(myOptions.powerCheckIntervalSec * 1000000);  
}
