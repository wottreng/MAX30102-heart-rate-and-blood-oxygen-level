#pragma once

class EEPROM_tools_c {
public:
    void init();

    String read_eeprom(int address);

    bool write_to_EEPROM(String data, int address);
};

extern EEPROM_tools_c EEPROM_tools;