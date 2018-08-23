#ifndef __duty_cycle_manager_h
#define __duty_cycle_manager_h

#include "common.h"
#include "time.h"

class DutyCycleManagerClass {
public:
#pragma pack(push, 1)
    struct Task {
        static const int kDescriptionMaxLength = 15;

        Valve valve;
        bool isEnabled;
        Seconds duration;
        char description[kDescriptionMaxLength + 1];
    };
#pragma pack(pop)

public:
    DutyCycleManagerClass();

    void loadState();

    void updateTask(const Task& task);
    const Task& task(int index) const { return _tasks[index]; }

    bool isDue() const;
    TimeInterval timeIntervalSinceLastCycle() const;
    TimeInterval timeIntervalTillNextCycle() const;
    TimeInterval cycleInterval() const { return _cycleInterval; }

    void run();
    void reset();
    void schedule(const TimeInterval& ti);
    void setCycleInterval(const TimeInterval& ti);

private:
    void loadTasks();
    void saveTasks();

private:
    Task _tasks[kNumOutputValves];
    CumulativeTime _lastCycleCumulativeTime;
    UnixTime _lastCycleUnixTime;
    bool _isScheduled;
    CumulativeTime _scheduledCumulativeTime;
    TimeInterval _cycleInterval;
};

extern DutyCycleManagerClass DutyCycleManager;

#endif // __duty_cycle_manager_h
