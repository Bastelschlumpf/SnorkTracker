/*
 *   DS2438TemperatureAndVoltage
 *
 *   This example demonstrates the use of the DS2438 Library to read temperature and
 *   voltage from a Dallas Semiconductor DS2438 battery monitor using the Arduino
 *   OneWire library.
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

#include <Arduino.h>
#include <OneWire.h>
#include <DS2438.h>

// define the Arduino digital I/O pin to be used for the 1-Wire network here
const uint8_t ONE_WIRE_PIN = 2;

// define the 1-Wire address of the DS2438 battery monitor here (lsb first)
uint8_t DS2438_address[] = { 0x26, 0x45, 0xe6, 0xf7, 0x00, 0x00, 0x00, 0x4e };

OneWire ow(ONE_WIRE_PIN);
DS2438 ds2438(&ow, DS2438_address);

void setup() {
    Serial.begin(9600);
    ds2438.begin();
}

void loop() {
    ds2438.update();
    if (ds2438.isError()) {
        Serial.println("Error reading from DS2438 device");
    } else {
        Serial.print("Temperature = ");
        Serial.print(ds2438.getTemperature(), 1);
        Serial.print("C, Channel A = ");
        Serial.print(ds2438.getVoltage(DS2438_CHA), 1);
        Serial.print("v, Channel B = ");
        Serial.print(ds2438.getVoltage(DS2438_CHB), 1);
        Serial.println("v.");
    }
    delay(500);
}
