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

enum EEPROMOffsets {
    kEEFirmwareVersion = 0,
    kEELastDutyCycleTime = kEEFirmwareVersion + sizeof(kFirmwareVersion),
    kEETasks = kEELastDutyCycleTime + sizeof(unsigned long),
    kEESize = kEETasks + kNumValves * 20
};

#define DEBUG 1

#if DEBUG
#include <HardwareSerial.h>
#define LOG Serial.print
#else
#define LOG
#endif

#endif // __common_h
