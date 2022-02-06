/*
Optical Heart Rate Detection and Blood Oxygen Levels with wifi integration
  By: Mark Wottreng
  Date: 24 January 2022

  References:
  https://github.com/sparkfun/MAX30105_Breakout
  https://github.com/aromring/MAX30102_by_RF
  https://github.com/Protocentral/Pulse_MAX30102
  https://www.forward.com.au/pfod/ESP8266/GPIOpins/ESP8266_01_pin_magic.html


  This is a demo to show the reading of heart rate and blood oxygen level
  This code is unverified and not calibrated, USE FOR DEVELOPEMENT ONLY

  It is best to attach the sensor to your finger using a rubber band or other tightening
  device. Humans are generally bad at applying constant pressure to a thing. When you
  press your finger against the sensor it varies enough to cause the blood in your
  finger to flow differently which causes the sensor readings to go wonky.

  Hardware Connections (Breakoutboard to ESP8266 NodeMCU):
  -3.3V power
  -GND
  -SDA => D1
  -SCL => D2

  Hardware Connections for MAX30102 to ESP8266-01 (1MB)
  -SDA => GPIO 0 or GPIO 1(TX)
  -SCL => GPIO 2 or GPIO 3(RX)

  LIMITATIONS:
    * GPIO 0 goes LOW on start up
    * GPIO 0 and 2 need to be HIGH to boot

  PINS:
    *GPIO: TX (pin 1), RX (pin 3), pin 0, pin 2
    *power: CH_PD,VCC = HIGH, GDN = LOW
    *LED_BUILTIN 1

  * NOTE: if reading are not consistent, some calibration may be required see max30102.h

*/

#include <Arduino.h>
#include <MAX30102_PulseOximeter.h>
//
#include <myEEPROM.h>
#include <myWifi.h>
#include <myData.h>

//
#define REPORTING_PERIOD_MS 100
// Sampling is tightly related to the dynamic range of the ADC.
// refer to the datasheet for further info
// #define SAMPLING_RATE                       MAX30102_SAMPRATE_100HZ

// The LEDs currents must be set to a level that avoids clipping and maximises the
// dynamic range
// #define IR_LED_CURRENT                      MAX30102_LED_CURR_50MA
// #define RED_LED_CURRENT                     MAX30102_LED_CURR_27_1MA

// The pulse width of the LEDs driving determines the resolution of
// the ADC (which is a Sigma-Delta).
// set HIGHRES_MODE to true only when setting PULSE_WIDTH to MAX30100_SPC_PW_1600US_16BITS
// #define PULSE_WIDTH                         MAX30102_SPC_PW_118US_16BITS
// #define HIGHRES_MODE                        true

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation

#define DEBUG true // turn on serial output

//
PulseOximeter pox; // pulse oximeter class declaration
//
uint32_t tsLastReport = 0;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    myData.heart_rate = int(pox.getHeartRate());
    myData.SpO2 = abs(pox.getSpO2());
#if DEBUG
    Serial.println("----");
    Serial.println(myData.heart_rate);
    Serial.println(myData.SpO2);
    Serial.println("----");
#endif
    tsLastReport = millis();
}

void setup()
{
#if DEBUG
    Serial.begin(115200);
#endif
    //
    EEPROM_tools.init(); // initialize eeprom
    //
    myWifi.init(); // read credentials from eeprom
    //
    myWifi.connect_Wifi(true); // try to connect to wifi or open AP

    // SDA and SCL two wire pin config (uncomment and change as needed, defaults are stored in myData.h)
    // myData.sda_pin = 5;
    // myData.scl_pin = 4;

    // Initialize the PulseOximeter instance
    if (!pox.begin())
    {
#if DEBUG
        // Failures are generally due to an improper I2C wiring, missing power supply or wrong target chip
        Serial.println("FAILED");
#endif
        while (1)
            ;
    }
    else
    {
#if DEBUG
        Serial.println("SUCCESS");
#endif
    }
    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop()
{
    pox.update(); // call to max30102 sensor for data
    myWifi.loop(); // respond to wifi requests

    if (millis() - tsLastReport > REPORTING_PERIOD_MS)
    {
        // due stuff here if needed
    }    
}
