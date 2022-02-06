/*
my Wifi lib

wifi functionality control functions

*/
#include <Arduino.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <string.h>
//#include <ESP8266HTTPClient.h>
//#include <WiFiClientSecure.h>

// custom libs
#include "myWifi.h"
#include <myEEPROM.h>
#include <myData.h>

#define DEBUG false

#define LED_pin 16

// local variables ---------------

// http server
ESP8266WebServer httpServer(80); // port 80 aka http

ESP8266HTTPUpdateServer httpUpdater; // update server
const String hostName = "esp_dev"; // access via browser: http://esp_dev.local/  or  http://192.168.10.1/ if in AP mode
// AP vars -----
IPAddress apIP(192, 168, 10, 1);
const String APssid = hostName;
const String APpswd = "";
const int chan = 1;
const int hidden = false;
const int max_con = 4;

// eeprom wifi credential location
#define wifiSSIDloc 0
#define wifiPSWDloc 20
//
//PulseOximeter local_pox;

// functions ==================================================================

// read wifi creds from eeprom
void myWifi_c::init()
{
    myWifi.WIFI_SSID = EEPROM_tools.read_eeprom(wifiSSIDloc); // string
    myWifi.WIFI_PASS = EEPROM_tools.read_eeprom(wifiPSWDloc);
#if DEBUG == true
    Serial.println("----");
    Serial.println(myWifi.WIFI_SSID);
#endif

}

// wifi handle client and update MDNS
void myWifi_c::loop()
{
    httpServer.handleClient();
    if(WiFi.status() == 3){ // if connected
        MDNS.update();
    }
}

// url not found
void notFound()
{ // when stuff after '/' is incorrect
    httpServer.send(200, "text/html", "not a endpoint"); // Send web page
}

// check wifi status and connect wifi or else create AP, return true if connected to wifi AP or false
bool myWifi_c::connect_Wifi(bool New_Connection)
{
    if (WiFi.status() != 3 || New_Connection == true) // if not conneted or new connection
    {
        if(myWifi.WIFI_SSID == "---"){ // if ssid not in eeprom, then open an AP hot spot
            WiFi.mode(WIFI_AP); // WIFI_AP
            WiFi.setOutputPower(16); // 20.5 -> 0 in .25 increments
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); //#
            WiFi.softAP(APssid, APpswd, chan, hidden, max_con);
            myWifi_c::web_server_config();

            return false; 
        }
        // wifi_set_sleep_type(LIGHT_SLEEP_T); // Auto modem sleep is default, light sleep for power savings
        WiFi.persistent(false);
        WiFi.setOutputPower(20);
        WiFi.mode(WIFI_STA); // WIFI_STA
        WiFi.hostname(hostName.c_str());
        WiFi.begin(myWifi.WIFI_SSID, myWifi.WIFI_PASS);
        const uint8_t maxTry = 10; // try this many times to connect
        uint8_t x = 0;
        while (x <= maxTry) {
            delay(800);
            uint8_t conStatus = WiFi.status();
            x += 1;
            if (conStatus == 3) { // connected to AP
                WiFi.mode(WIFI_STA);
                x = 100;
#if DEBUG
                digitalWrite(LED_pin, HIGH); // off
#endif

            } else if (conStatus != 3 && x == maxTry) { // couldnt connect to ssid, go into access point mode
                WiFi.mode(WIFI_AP); // WIFI_AP
                WiFi.setOutputPower(16); // 20.5 -> 0 in .25 increments
                WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); //#
                WiFi.softAP(APssid, APpswd, chan, hidden, max_con);
#if DEBUG
                digitalWrite(LED_pin, LOW); // on
                // Serial.println("no AP connection, starting hotspot");
#endif
            }
        }
        myWifi_c::web_server_config();
        return true;
    }
    return false;
}

// for api output
void jsonData()
{
    // unsigned long lastOffsec = (millis() - lastOff) / 1000;
    String output = "{"
                    "\"Temp\":\"" + String(0) + "\","
                    "\"HR\":\"" + String(myData.heart_rate) + "\","
                    "\"SpO2\":\"" + String(myData.SpO2) + "\","
                    //
                    "\"Wifi_SSID\":\"" + myWifi.WIFI_SSID + "\",";

    uint8_t mode = WiFi.getMode();
    if (mode == 1)
        output += "\"Wifi_Mode\":\"Station\",";
    else if (mode == 2)
        output += "\"Wifi_Mode\":\"Access Point\",";
    else if (mode == 3)
        output += "\"Wifi_Mode\":\"STA_AP\",";
    else if (mode == 0)
        output += "\"Wifi_Mode\":\"Off\",";

    output += "\"Wifi_Power_dBm\":\"20\","
                "\"Wifi_PhyMode\":\""  + String(WiFi.getPhyMode()) + "\"," // 3:
                "\"Wifi_Sleep_Mode\":\"" + String(WiFi.getSleepMode()) + "\"," // 1: light_sleep
                "\"Wifi_IP_addr\":\"" + WiFi.localIP().toString() + "\",";

    uint8_t conStatus = WiFi.status();
    if (conStatus == 0)
        output += "\"Wifi_Status\":\"Idle\",";
    else if (conStatus == 3)
        output += "\"Wifi_Status\":\"Connected\",";
    else if (conStatus == 1)
        output += "\"Wifi_Status\":\"not connected\",";
    else if (conStatus == 7)
        output += "\"Wifi_Status\":\"Disconnected\",";
    else
        output += "\"Wifi_Status\":\"" + String(conStatus) + "\",";

    output += "\"Wifi_Signal_dBm\":\"" + String(WiFi.RSSI()) + "\","
                "\"Core Frequency MHz\":\"" + String(ESP.getCpuFreqMHz()) + "\","
                "\"reason last reset\":\""  + ESP.getResetReason() + "\","
                "\"free HEAP\":\"" + String(ESP.getFreeHeap()) + "\","
                "\"ESP ID\":\""  + String(ESP.getChipId()) + "\","
                "\"sketch size\":\""  + String(ESP.getSketchSize()) + "\","
                "\"version\":\"2.0\""
                "}";

    httpServer.send(200, "application/json", output);
}

// direct api call
void argData()
{
    // input wifi AP creds via api
    if (httpServer.arg("SSID") != "" && httpServer.arg("PASS") != "") {
        // This routine is executed when you change wifi AP

        myWifi.WIFI_SSID = httpServer.arg("SSID");
        myWifi.WIFI_PASS = httpServer.arg("PASS"); // string
        EEPROM_tools.write_to_EEPROM(myWifi.WIFI_SSID, wifiSSIDloc);
        EEPROM_tools.write_to_EEPROM(myWifi.WIFI_PASS, wifiPSWDloc);

        jsonData();
        httpServer.stop();
        myWifi.connect_Wifi(true);
        return;
    }
    //
    jsonData();
}

//
void reboot()
{
    ESP.restart();
}

// web server config
void myWifi_c::web_server_config()
{
    if (WiFi.getMode() == 1)
        MDNS.begin(hostName.c_str());

    //------server--------------------
    // server.stop();
    httpServer.onNotFound(notFound); // after ip address /
    httpServer.on("/", HTTP_GET, argData);
    httpServer.on("/reboot", HTTP_GET, reboot);
    httpUpdater.setup(&httpServer);
    httpServer.begin();
    MDNS.addService("http", "tcp", 80);
}
//
myWifi_c myWifi;