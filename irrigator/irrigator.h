#ifndef __irrigator_h
#define __irrigator_h

#include "common.h"

class IrrigatorClass {
public:
    struct Task {
        Valve valve;
        Seconds duration;
    };

public:
    IrrigatorClass();

    void openValve(Valve valve);
    void closeValve(Valve valve);
    void performTask(Task& task);
    void reset();
    
    const bool isBusy() const { return _openValvesMask != 0; }

private:
    void prepareValveForOpening(Valve valve);
    uint8_t pinForValve(Valve valve);
    void logOpenMask();

private:
    static const Milliseconds kValveOpenTransientTime = 500;
    static const Milliseconds kValveCloseTransientTime = 500;

    uint8_t _openValvesMask;
};

static IrrigatorClass Irrigator;

#endif // __irrigator_h
