/*
 *   DS2438.h
 *
 *   by Joe Bechter
 *
 *   (C) 2012, bechter.com
 *
 *   All files, software, schematics and designs are provided as-is with no warranty.
 *   All files, software, schematics and designs are for experimental/hobby use.
 *   Under no circumstances should any part be used for critical systems where safety,
 *   life or property depends upon it. You are responsible for all use.
 *   You are free to use, modify, derive or otherwise extend for your own non-commercial purposes provided
 *       1. No part of this software or design may be used to cause injury or death to humans or animals.
 *       2. Use is non-commercial.
 *       3. Credit is given to the author (i.e. portions © bechter.com), and provide a link to the original source.
 *
 */

#ifndef DS2438_h
#define DS2438_h

#include <Arduino.h>
#include <OneWire.h>

#define DS2438_TEMPERATURE_CONVERSION_COMMAND 0x44
#define DS2438_VOLTAGE_CONVERSION_COMMAND 0xb4
#define DS2438_WRITE_SCRATCHPAD_COMMAND 0x4e
#define DS2438_COPY_SCRATCHPAD_COMMAND 0x48
#define DS2438_READ_SCRATCHPAD_COMMAND 0xbe
#define DS2438_RECALL_MEMORY_COMMAND 0xb8
#define DS2438_PAGE_0 0x00

#define DS2438_CHA 0
#define DS2438_CHB 1

#define DS2438_MODE_CHA 0x01
#define DS2438_MODE_CHB 0x02
#define DS2438_MODE_TEMPERATURE 0x04

#define DS2438_TEMPERATURE_DELAY 10
#define DS2438_VOLTAGE_CONVERSION_DELAY 8

class DS2438 {
    public:
        DS2438(OneWire *ow, uint8_t *address);
        void begin(uint8_t mode=(DS2438_MODE_CHA | DS2438_MODE_CHB | DS2438_MODE_TEMPERATURE));
        void update();
        double getTemperature();
        float getVoltage(int channel=DS2438_CHA);
        boolean isError();
        unsigned long getTimestamp();
    private:
        OneWire *_ow;
        uint8_t *_address;
        uint8_t _mode;
        double _temperature;
        float _voltageA;
        float _voltageB;
        unsigned long _timestamp;
        boolean _error;
        boolean startConversion(int channel, boolean doTemperature);
        boolean selectChannel(int channel);
        void writePageZero(uint8_t *data);
        boolean readPageZero(uint8_t *data);
};

#endif
