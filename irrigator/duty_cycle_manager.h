#ifndef __duty_cycle_manager_h
#define __duty_cycle_manager_h

#include "common.h"
#include "time.h"

class DutyCycleManagerClass {
public:
    struct Task {
        static const int kDescriptionMaxLength = 15;

        bool isEnabled;
        char description[kDescriptionMaxLength + 1];
        Valve valve;
        Milliseconds duration;
    };

public:
    DutyCycleManagerClass();

    void updateTask(const Task& task);
    const Task& task(int index) const { return _tasks[index]; }

    bool isDue() const;
    TimeInterval timeIntervalSinceLastCycle() const;
    TimeInterval timeIntervalTillNextCycle() const;

    void run();
    void reset();

private:
    void loadTasks();
    void saveTasks();

private:
    Task _tasks[kNumValves];
    DeviceTime _lastCycleDeviceTime;
    UnixTime _lastCycleUnixTime;
};

extern DutyCycleManagerClass DutyCycleManager;

#endif // __duty_cycle_manager_h
