#ifndef __clock_h
#define __clock_h

#include "time.h"

class ClockClass {
public:
    ClockClass();

    bool isIsolated() const { return _startupTime == UnixTime::distantPast(); }
    bool sync();

    DeviceTime deviceTime();
    UnixTime unixTime() {
        if (isIsolated()) {
            return UnixTime::distantPast();
        }
        TimeInterval interval = deviceTime().timeIntervalSinceReferenceTime();
        return _startupTime + interval;
    }

    UnixTime unixTimeFromDeviceTime(const DeviceTime& dt) {
        if (isIsolated()) {
            return UnixTime::distantPast();
        }
        TimeInterval interval = dt.timeIntervalSinceReferenceTime();
        return _startupTime + interval;
    }

private:
    DeviceTime _lastSuccessfulSyncTime;
    DeviceTime _lastSyncTrialTime;
    UnixTime _startupTime;

    uint16_t _systemMillisOverflow;
    unsigned long _lastSeenSystemMillis;
};

extern ClockClass Clock;


#endif // __clock_h


// save devicetime every N minutes to EEPROM to preserve uptime for after an outage?
