#ifndef __moisture_logger_h
#define __moisture_logger_h

#include "time.h"

class MoistureLoggerClass {
public:
    MoistureLoggerClass();
    int sample();
    bool submitToIOTPlotter(int value);
    bool submitToThingspeak(int value);

private:
    int _minValue;
    int _maxValue;
    int _lastValue;
    CumulativeTime _lastSampleCumulativeTime;
};

extern MoistureLoggerClass MoistureLogger;

#endif // __moisture_logger_h
