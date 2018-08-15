#ifndef __time_h
#define __time_h

#include <WString.h>
#include <stdint.h>

template <typename T> class Time;

class TimeInterval {
public:
    static const int kNumFractionBits = 16;
    static const uint64_t kFractionMask = (uint64_t(1) << kNumFractionBits) - 1;
    static const uint64_t kSignMask = uint64_t(1) << 63;
public:
    static TimeInterval withSeconds(int32_t sec) {
        return TimeInterval(int64_t(sec) << kNumFractionBits);
    }

    static TimeInterval withMilliseconds(int32_t msec) {
        return TimeInterval((int64_t(msec) << kNumFractionBits) / 1000);
    }

    static TimeInterval neverInThePast() {
        return TimeInterval(~uint64_t(0));
    }

    static TimeInterval neverInTheFuture() {
        return TimeInterval(~uint64_t(0) | kSignMask);
    }

    int32_t seconds() const {
        return _ticks >> kNumFractionBits;
    }

    int16_t fractionInMilliseconds() const {
        int64_t signedFraction = _ticks & (kFractionMask | kSignMask);
        return (signedFraction * 1000) >> (kNumFractionBits - 1);
    }

    TimeInterval operator+(const TimeInterval& other) const {
        TimeInterval ti = *this;
        ti += other;
        return ti;
    }

    TimeInterval& operator+=(const TimeInterval& ti) {
        _ticks += ti._ticks;
        return *this;
    }

    TimeInterval operator-(const TimeInterval& other) const {
        TimeInterval ti = *this;
        ti -= other;
        return ti;
    }

    TimeInterval& operator-=(const TimeInterval& ti) {
        _ticks -= ti._ticks;
        return *this;
    }

    bool operator==(const TimeInterval& other) const {
        return _ticks == other._ticks;
    }

    bool operator!=(const TimeInterval& other) const {
        return _ticks != other._ticks;
    }

    bool operator<(const TimeInterval& other) const {
        return _ticks < other._ticks;
    }

    bool operator<=(const TimeInterval& other) const {
        return _ticks <= other._ticks;
    }

    bool operator>(const TimeInterval& other) const {
        return _ticks > other._ticks;
    }

    bool operator>=(const TimeInterval& other) const {
        return _ticks >= other._ticks;
    }

    String toHumanReadableString() const;

private:
    TimeInterval(uint64_t ticks): _ticks(ticks) {} 
    
public:
    int64_t _ticks;

    template <typename T> friend class Time;
};

// generic time class
template <typename T>
class Time {
public:
    static T distantPast() { return Time(TimeInterval::neverInThePast()._ticks); }
    static T distantFuture() { return Time(TimeInterval::neverInTheFuture()._ticks); }

    uint32_t seconds() const { return _interval.seconds(); }
    uint16_t fractionInMilliseconds() const { return _interval.fractionInMilliseconds(); }

    T operator+(const TimeInterval& ti) const {
        T t = *this;
        t._interval += ti;
        return t;
    }

    T& operator+=(const TimeInterval& ti) {
        _interval += ti;
        return *this;
    }

    T operator-(const TimeInterval& ti) const {
        T t = *this;
        t._interval -= ti;
        return t;
    }

    T& operator-=(const TimeInterval& ti) {
        _interval -= ti;
        return *this;
    }

    bool operator==(const T& other) const {
        return _interval == other._interval;
    }

    bool operator!=(const T& other) const {
        return _interval != other._interval;
    }

    bool operator<(const T& other) const {
        return _interval < other._interval;
    }
    
    bool operator<=(const T& other) const {
        return _interval <= other._interval;
    }

    bool operator>(const T& other) const {
        return _interval > other._interval;
    }

    bool operator>=(const T& other) const {
        return _interval >= other._interval;
    }

    const TimeInterval& timeIntervalSinceReferenceTime() const { return _interval; }

    TimeInterval timeIntervalSince(const T& other) const {
        return _interval - other._interval;
    }

protected:
    Time(uint64_t ticks): _interval(ticks) {}

private:
    TimeInterval _interval;
};


// time since the Unix Epoch
class UnixTime: public Time<UnixTime> {
public:
    explicit UnixTime(uint32_t timestamp):
        Time(uint64_t(timestamp) << TimeInterval::kNumFractionBits) {
    }

    UnixTime(const Time<UnixTime>& base): Time(base) {}
};

// time since last boot
class DeviceTime: public Time<DeviceTime> {
public:
    explicit DeviceTime(unsigned long msec, uint16_t overflow = 0):
        Time((((uint64_t(overflow) << 32) | msec) << TimeInterval::kNumFractionBits) / 1000) {
    }

    DeviceTime(const Time<DeviceTime>& base): Time(base) {}
};

// approximate accumulated uptime since first boot
class CumulativeTime: public Time<CumulativeTime> {
public:
    explicit CumulativeTime(const DeviceTime& localTime, 
                            const TimeInterval& previousUptime = TimeInterval::withSeconds(0)):
        Time(({
            TimeInterval interval = localTime.timeIntervalSinceReferenceTime() + previousUptime;
            (uint64_t)interval._ticks;
        })) {
    }

    CumulativeTime(const Time<CumulativeTime>& base): Time(base) {}
};

#endif // __time_h
