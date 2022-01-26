# MAX30102-heart-rate-and-blood-oxygen-level
MAX30102 sensor integration with ESP8266 for wifi/bluetooth capabilities

Optical Heart Rate Detection and Blood Oxygen Levels
By: Mark Wottreng
Date: 24 January 2022

Code References:
https://github.com/sparkfun/MAX30105_Breakout
https://github.com/aromring/MAX30102_by_RF
https://github.com/Protocentral/Pulse_MAX30102

Hardware References:
ESP8266 NodeMCU: https://amzn.to/3o2HhWl
MAX30102 sensor: https://amzn.to/3u3wEGQ

Data Sheets:
https://datasheets.maximintegrated.com/en/ds/MAX30102.pdf
https://components101.com/development-boards/nodemcu-esp8266-pinout-features-and-datasheet

bit manipulation code reference:
https://www.learncpp.com/cpp-tutorial/bit-manipulation-with-bitwise-operators-and-bit-masks/


This is a demo to show the reading of heart rate and blood oxygen level
This code is unverified and not calibrated, USE FOR DEVELOPEMENT ONLY

It is best to attach the sensor to your finger using a rubber band or other tightening
device. Constant pressure is required to get a good reading. When the pressure against
your finger varies, the blood in your finger flows differently which causes the sensor 
readings to vary.

Hardware Connections (MAX30102 Breakoutboard to ESP8266 NodeMCU):
-VIN => 3.3V power
-GND => GND
-SDA => D1
-SCL => D2
-INT => D6

* NOTE: if reading are not consistent, some calibration may be required
        see maxim_max30102_init() in /lib/max30102/max30102.cpp