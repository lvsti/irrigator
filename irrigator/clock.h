#ifndef __clock_h
#define __clock_h

#include "time.h"

class ClockClass {
public:
    ClockClass();

    bool isIsolated() const { return _startupTime == UnixTime::distantPast(); }
    bool sync();

    void loadUptime();
    void saveUptime();

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

    UnixTime unixTimeFromCumulativeTime(const CumulativeTime& ct) {
        if (isIsolated()) {
            return UnixTime::distantPast();
        }
        TimeInterval interval = ct.timeIntervalSinceReferenceTime();
        return _firstStartupTime + interval;
    }

    CumulativeTime cumulativeTime() {
        return CumulativeTime(deviceTime(), _previousUptime);
    }

    CumulativeTime cumulativeTimeFromDeviceTime(const DeviceTime& dt) {
        return CumulativeTime(dt, _previousUptime);
    }

private:
    DeviceTime _lastSuccessfulSyncTime;
    DeviceTime _lastSyncTrialTime;
    DeviceTime _lastUptimeSaveTime;
    UnixTime _startupTime;
    UnixTime _firstStartupTime;
    TimeInterval _previousUptime;

    uint16_t _systemMillisOverflow;
    unsigned long _lastSeenSystemMillis;
};

extern ClockClass Clock;

#endif // __clock_h
