#ifndef __common_h
#define __common_h

#include <stdint.h>

extern const uint16_t kFirmwareVersion;

// NodeMCU pin mapping
extern const uint8_t pinD[];
extern const uint8_t pinA[];

extern const uint8_t pinDForValve[];

typedef enum {
    kValveOutput1 = 0,
    kValveOutput2,
    kValveOutput3,
    kValveOutput4,

    kNumOutputValves,

    kValveMaster = kNumOutputValves,
};
typedef uint8_t Valve;

static const Valve outputValves[] = {kValveOutput1, kValveOutput2, kValveOutput3, kValveOutput4};

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
EEPROM_CELL_SIZE(kEETasks, kNumOutputValves * 20)
EEPROM_CELL_TYPE(kEEDutyCycleIntervalSeconds, uint32_t)

EEPROM_LAYOUT_END


#define DEBUG 1

#if DEBUG
#include <HardwareSerial.h>
#define LOG(...) Serial.print(__VA_ARGS__)
#else
#define LOG(...)
#endif

#endif // __common_h
