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
  * @file Serial.h
  *
  * Class to hook the serial communication and store the information in the console stringlist.
  */

/** 
  * Helper class to hook the SoftwareSerial calls to log the information for the console. 
  */
class MySerial : public SoftwareSerial
{
protected:
   char        inData[255];  //!< Helper data for a serial write.
   int         inIdx;        //!< How many bytes are written.
   char        outData[255]; //!< Helper data for a serial read.
   int         outIdx;       //!< How many bytes are read.
   StringList &logInfos;     //!< Hook pointer for the data logging.
   bool       &debug;        //!< Enable or disable the hooking.

public:
   MySerial(StringList &li, bool &dbg, uint8_t receivePin, uint8_t transmitPin, bool inverse_logic = false);

   virtual int    read();
   virtual size_t write(uint8_t byte);
};

/* ******************************************** */

/** Constructor */
MySerial::MySerial(StringList &li, bool &dbg, uint8_t receivePin, uint8_t transmitPin, bool inverse_logic /*= false*/)
   : SoftwareSerial(receivePin, transmitPin, inverse_logic)
   , inIdx(0)
   , outIdx(0)
   , logInfos(li)
   , debug(dbg)
{
}

/** Virtual function call on read operations.
  * We check for incomming calls. 
  */
int MySerial::read()
{
   int ret = SoftwareSerial::read();

   if (ret >= 0 && debug) {
      char c = (char) ret;

      if (c != '\r' && c != '\n') {
         if (inIdx < 250) {
            inData[inIdx++] = c;
         }
      } else {
         if (inIdx > 0) {
            inData[inIdx] = 0;
            logInfos.addTail("< " + (String) inData);
         }
         inIdx = 0;
      }
   }

   return ret;
}

/** Virtual function call on write operations */
size_t MySerial::write(uint8_t byte)
{
   size_t ret = SoftwareSerial::write(byte);

   if (debug) {
      char c = (char)byte;

      if (c != '\r' && c != '\n') {
         if (outIdx < 250) {
            outData[outIdx++] = c;
         }
      } else {
         if (outIdx > 0) {
            outData[outIdx] = 0;
            logInfos.addTail("> " + (String)outData);
         }
         outIdx = 0;
      }
   }
   return ret;
}
