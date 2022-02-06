#pragma once

#include <Arduino.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <MAX30102_PulseOximeter.h>

class myWifi_c
{
public:
    // wifi 
    String WIFI_SSID;
    String WIFI_PASS;

    // run in setup loop
    void init();

    // run in main loop
    void loop();

    // try to connect to wifi AP, if none then create hotspot AP
    bool connect_Wifi(bool New_Connection);

    void web_server_config();

};

extern myWifi_c myWifi;
