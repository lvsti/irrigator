#ifndef __common_h
#define __common_h

#include <stdint.h>

extern const uint16_t kFirmwareVersion;

// NodeMCU pin mapping
extern const uint8_t pinD[];

static const int kNumValves = 6;
extern const uint8_t pinDForValve[];

typedef uint8_t Valve;
typedef uint16_t Milliseconds;
typedef uint16_t Seconds;

#define EEPROM_LAYOUT_BEGIN \
    enum EEPROMOffsets { \
    kEE_LAYOUT_BEGIN = -1,

#define EEPROM_LAYOUT_END \
    kEESize \
    };

#define EEPROM_CELL_TYPE(__alias__, __type__) \
    __alias__, \
    __alias__##_END = __alias__ + sizeof(__type__) - 1,

#define EEPROM_CELL_SIZE(__alias__, __size__) \
    __alias__, \
    __alias__##_END = __alias__ + (__size__) - 1,



EEPROM_LAYOUT_BEGIN

EEPROM_CELL_TYPE(kEEFirmwareVersion, uint16_t)
EEPROM_CELL_TYPE(kEELastDutyCycleCumulativeTimeSeconds, uint32_t)
EEPROM_CELL_TYPE(kEELastDutyCycleUnixTimeSeconds, uint32_t)
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
