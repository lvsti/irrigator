#ifndef __time_h
#define __time_h

#include <stdint.h>

template <typename T> class Time;

class TimeInterval {
public:
    static const int kFractionResolution = 16;
public:
    static TimeInterval withSeconds(int32_t sec) {
        return TimeInterval(sec << kFractionResolution);
    }

    static TimeInterval withMilliseconds(int32_t msec) {
        return TimeInterval((msec << kFractionResolution) / 1000);
    }

    int32_t seconds() const {
        return ((int32_t)_raw.components.sign << 31) | (int32_t)_raw.components.seconds;
    }

    int16_t milliseconds() const {
        return ((int16_t)_raw.components.sign << 15) | ((int32_t)_raw.components.fraction * 1000) >> kFractionResolution;
    }

    TimeInterval operator+(const TimeInterval& other) const {
        TimeInterval ti = *this;
        ti += other;
        return ti;
    }

    TimeInterval& operator+=(const TimeInterval& ti) {
        _raw.ticks += ti._raw.ticks;
        return *this;
    }

    TimeInterval operator-(const TimeInterval& other) const {
        TimeInterval ti = *this;
        ti -= other;
        return ti;
    }

    TimeInterval& operator-=(const TimeInterval& ti) {
        _raw.ticks -= ti._raw.ticks;
        return *this;
    }

    bool operator==(const TimeInterval& other) const {
        return _raw.ticks == other._raw.ticks;
    }

    bool operator!=(const TimeInterval& other) const {
        return _raw.ticks != other._raw.ticks;
    }

    bool operator<(const TimeInterval& other) const {
        return _raw.ticks < other._raw.ticks;
    }

    bool operator<=(const TimeInterval& other) const {
        return _raw.ticks <= other._raw.ticks;
    }

    bool operator>(const TimeInterval& other) const {
        return _raw.ticks > other._raw.ticks;
    }

    bool operator>=(const TimeInterval& other) const {
        return _raw.ticks >= other._raw.ticks;
    }

private:
    TimeInterval(uint64_t ticks) {
        _raw.ticks = ticks;
    } 
    
private:
    union {
        int64_t ticks;
        struct {
            unsigned sign:1;
            unsigned :(32 - kFractionResolution);
            unsigned seconds:31;
            unsigned fraction:kFractionResolution;
        } components;
    } _raw;

    template <typename T> friend class Time;
};

// generic time class
template <typename T>
class Time {
public:
    static T distantPast() { return Time(~0); }
    static T distantFuture() { return Time(~0 ^ (1 << 63)); }

    uint32_t seconds() const { return _interval.seconds(); }
    uint16_t milliseconds() const { return _interval.milliseconds(); }

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
        Time((uint64_t)timestamp << TimeInterval::kFractionResolution) {
    }

    UnixTime(const Time<UnixTime>& base): Time(base) {}
};

// time since startup
class DeviceTime: public Time<DeviceTime> {
public:
    explicit DeviceTime(unsigned long msec, uint16_t overflow = 0):
        Time((((((uint64_t)overflow) << 32) | msec) << TimeInterval::kFractionResolution) / 1000) {
    }

    DeviceTime(const Time<DeviceTime>& base): Time(base) {}
};


#endif // __time_h
