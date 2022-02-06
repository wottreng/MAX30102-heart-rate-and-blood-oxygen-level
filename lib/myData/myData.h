#pragma once

class data_c {
public:
    int heart_rate;
    uint8_t SpO2;
    float temp;
    uint8_t sda_pin = 5; // D1 on NodeMCU
    uint8_t scl_pin = 4; // D2 on NodeMCU
};

extern data_c myData;