#ifndef __common_h
#define __common_h

#include <stdint.h>

extern const uint16_t kFirmwareVersion;

// NodeMCU pin mapping
extern const uint8_t pinD[];

static const int kNumValves = 6;
extern const uint8_t pinDForValve[];

typedef uint8_t Valve;
typedef unsigned int Milliseconds;

#define EEPROM_LAYOUT_BEGIN \
    enum EEPROMOffsets { \
    kEE_LAYOUT_BEGIN = -1,

#define EEPROM_LAYOUT_END \
    };

#define EEPROM_CELL_TYPE(__alias__, __type__) \
    __alias__,
    __alias__##_END = alias + sizeof(__type__) - 1,

#define EEPROM_CELL_SIZE(__alias__, __size__) \
    __alias__,
    __alias__##_END = alias + (__size__) - 1,



EEPROM_LAYOUT_BEGIN

EEPROM_CELL_TYPE(kEEFirmwareVersion, uint16_t)
EEPROM_CELL_TYPE(kEELastDutyCycleDeviceTime, uint64_t)
EEPROM_CELL_TYPE(kEELastDutyCycleUnixTime, uint32_t)
EEPROM_CELL_TYPE(kEEPreviousUptimeSeconds, uint32_t)
EEPROM_CELL_SIZE(kEETasks, kNumValves * 20)

EEPROM_LAYOUT_END


#define DEBUG 1

#if DEBUG
#include <HardwareSerial.h>
#define LOG Serial.print
#else
#define LOG
#endif

#endif // __common_h
