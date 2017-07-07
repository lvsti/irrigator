#ifndef __eeprom_ext_h
#define __eeprom_ext_h

#include <EEPROM.h>

template<typename T> 
T& get(EEPROMClass& e, int addr, T& t) {
    uint8_t* ptr = (uint8_t*)&t;
    for (int count = sizeof(T); count > 0; --count, ++addr) {
        *ptr++ = e.read(addr);
    }
    return t;
}

template<typename T>
const T& put(EEPROMClass& e, int addr, const T& t) {
    const uint8_t* ptr = (const uint8_t*)&t;
    for (int count = sizeof(T); count > 0; --count, ++addr) {
        e.write(addr, *ptr++);
    }
    return t;
}

#endif // __eeprom_ext_h
