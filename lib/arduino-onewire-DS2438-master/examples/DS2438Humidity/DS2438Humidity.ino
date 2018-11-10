/*
 *   DS2438Humidity
 *
 *   This example demonstrates the use of the DS2438 Library and the Arduino OneWire library
 *   to read a relative humidity sensor that uses a Dallas Semiconductor DS2438 battery monitor.
 *   This example is for a 1-Wire device similar to the HobbyBoards H3-R1-A humidity sensor.
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

float temperature;
float heatindex;
float dewpoint;
float humidity;

void setup() {
    Serial.begin(9600);
    ds2438.begin();
}

void loop() {
    ds2438.update();
    if (ds2438.isError() || ds2438.getVoltage(DS2438_CHA) == 0.0) {
        Serial.println("Error reading from DS2438 device");
    } else {
        temperature = ds2438.getTemperature();
        heatindex = temperature;
        float rh = (ds2438.getVoltage(DS2438_CHA) / ds2438.getVoltage(DS2438_CHB) - 0.16) / 0.0062;
        humidity = (float)(rh / (1.0546 - 0.00216 * temperature));
        if (humidity < 0.0) {
            humidity = 0.0;
        } else if (humidity > 100.0) {
            humidity = 100.0;
        }
        float tempK = temperature + 273.15;
        dewpoint = tempK / ((-0.0001846 * log(humidity / 100.0) * tempK) + 1.0) - 273.15;
        if (temperature >= 26.7 && humidity >= 40.0) {
            float t = temperature * 9.0 / 5.0 + 32.0; // heat index formula assumes degF
            rh = humidity;
            float heatindexF = -42.38 + 2.049 * t + 10.14 * rh + -0.2248 * t * rh + -0.006838 * t * t
                               + -0.05482 * rh * rh + 0.001228 * t * t * rh + 0.0008528 * t * rh * rh
                               + -0.00000199 * t * t * rh * rh;
            heatindex = (heatindexF - 32.0) * 5.0 / 9.0;
        }
        if (heatindex < temperature)
            heatindex = temperature;
    }
    Serial.print("Temperature = ");
    Serial.print(temperature, 1);
    Serial.print("C (");
    Serial.print(temperature * 9.0 / 5.0 + 32.0, 1);
    Serial.print("F), Heat Index = ");
    Serial.print(heatindex, 1);
    Serial.print("C (");
    Serial.print(heatindex * 9.0 / 5.0 + 32.0, 1);
    Serial.print("F), Dewpoint = ");
    Serial.print(dewpoint, 1);
    Serial.print("C (");
    Serial.print(dewpoint * 9.0 / 5.0 + 32.0, 1);
    Serial.print("F), Relative Humidity = ");
    Serial.print(humidity, 0);
    Serial.println("%.");
    delay(500);
}
