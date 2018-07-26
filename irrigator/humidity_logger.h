#ifndef __humidity_logger_h
#define __humidity_logger_h

#include "time.h"

class HumidityLoggerClass {
public:
    HumidityLoggerClass();
    int sample();
    bool submitToIOTPlotter(int value);

private:
    int _minValue;
    int _maxValue;
    int _lastValue;
    CumulativeTime _lastSampleCumulativeTime;
};

extern HumidityLoggerClass HumidityLogger;

#endif // __humidity_logger_h
