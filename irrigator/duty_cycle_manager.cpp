#include "duty_cycle_manager.h"

#include <EEPROM.h>
#include "clock.h"
#include "irrigator.h"

static const uint32_t kDutyCycleIntervalSeconds = 60 * 60 * 24;

DutyCycleManagerClass DutyCycleManager;

DutyCycleManagerClass::DutyCycleManagerClass():
    _lastCycleCumulativeTime(CumulativeTime::distantPast()),
    _lastCycleUnixTime(UnixTime::distantPast()) {
}

void DutyCycleManagerClass::loadState() {
    LOG(F("[DutyCycleManager] Last cycle run time (stored):\n"));

    uint32_t seconds = 0;
    EEPROM.get(kEELastDutyCycleCumulativeTimeSeconds, seconds);
    _lastCycleCumulativeTime = CumulativeTime(Clock.deviceTime(), TimeInterval::withSeconds(seconds));
    LOG(String(F("[DutyCycleManager]  - cumulative uptime: ")) + 
        _lastCycleCumulativeTime.timeIntervalSinceReferenceTime().toHumanReadableString() + 
        "\n");

    EEPROM.get(kEELastDutyCycleUnixTimeSeconds, seconds);
    _lastCycleUnixTime = UnixTime(seconds);
    LOG(String(F("[DutyCycleManager]  - network time: ")) + 
        String(_lastCycleUnixTime.timeIntervalSinceReferenceTime().seconds()) + 
        "\n");

    loadTasks();
}

bool DutyCycleManagerClass::isDue() const {
    int32_t seconds = timeIntervalTillNextCycle().seconds();
    LOG(String(F("[DutyCycleManager] next cycle should begin in ")) + String(seconds) + " sec\n");
    return seconds <= 0;
}

TimeInterval DutyCycleManagerClass::timeIntervalSinceLastCycle() const {
    CumulativeTime now = Clock.cumulativeTime();
    return now.timeIntervalSince(_lastCycleCumulativeTime);
}

TimeInterval DutyCycleManagerClass::timeIntervalTillNextCycle() const {
    DeviceTime localTime = Clock.deviceTime();
    if (Clock.isIsolated()) {
        CumulativeTime dueTime = _lastCycleCumulativeTime + TimeInterval::withSeconds(kDutyCycleIntervalSeconds);
        return dueTime.timeIntervalSince(Clock.cumulativeTimeFromDeviceTime(localTime));
    }

    UnixTime lastCycleUnixTime = _lastCycleUnixTime;
    if (lastCycleUnixTime == UnixTime::distantPast()) {
        lastCycleUnixTime = Clock.unixTimeFromCumulativeTime(_lastCycleCumulativeTime);
    }

    UnixTime dueTime = lastCycleUnixTime + TimeInterval::withSeconds(kDutyCycleIntervalSeconds);
    return dueTime.timeIntervalSince(Clock.unixTimeFromDeviceTime(localTime));
}

void DutyCycleManagerClass::run() {
    LOG(F("[DutyCycleManager] Starting duty cycle.\n"));
    DeviceTime cycleRunTime = Clock.deviceTime();

    for (int i = 0; i < kNumOutputValves; ++i) {
        const Task& task = _tasks[i];
        if (task.isEnabled) {
            IrrigatorClass::Task t;
            t.valve = task.valve;
            t.duration = task.duration;
            Irrigator.performTask(t);
        } 
        else {
            LOG(String(F("[DutyCycleManager] skipping task for valve ")) + String(task.valve) + String(F(": disabled\n")));
        }
    }

    _lastCycleCumulativeTime = Clock.cumulativeTimeFromDeviceTime(cycleRunTime);
    uint32_t seconds = _lastCycleCumulativeTime.timeIntervalSinceReferenceTime().seconds();
    EEPROM.put(kEELastDutyCycleCumulativeTimeSeconds, seconds);

    Clock.saveUptime();

    if (!Clock.isIsolated()) {
        _lastCycleUnixTime = Clock.unixTimeFromDeviceTime(cycleRunTime);
        seconds = _lastCycleUnixTime.timeIntervalSinceReferenceTime().seconds();
        EEPROM.put(kEELastDutyCycleUnixTimeSeconds, seconds);
    }

    LOG(F("[DutyCycleManager] Duty cycle finished.\n"));
}

void DutyCycleManagerClass::reset() {
    _lastCycleCumulativeTime = Clock.cumulativeTime();
    uint32_t seconds = _lastCycleCumulativeTime.timeIntervalSinceReferenceTime().seconds();
    EEPROM.put(kEELastDutyCycleCumulativeTimeSeconds, seconds);

    Clock.saveUptime();

    _lastCycleUnixTime = Clock.unixTimeFromCumulativeTime(_lastCycleCumulativeTime);
    seconds = _lastCycleUnixTime.timeIntervalSinceReferenceTime().seconds();
    EEPROM.put(kEELastDutyCycleUnixTimeSeconds, _lastCycleUnixTime);
}

void DutyCycleManagerClass::updateTask(const Task& task) {
    _tasks[task.valve] = task;
    saveTasks();
}

void DutyCycleManagerClass::loadTasks() {
    int addr = kEETasks;

    for (int i = 0; i < kNumOutputValves; ++i) {
        EEPROM.get(addr, _tasks[i]);
        _tasks[i].valve = outputValves[i];
        addr += sizeof(Task);
    }
}

void DutyCycleManagerClass::saveTasks() {
    int addr = kEETasks;

    for (int i = 0; i < kNumOutputValves; ++i) {
        EEPROM.put(addr, _tasks[i]);
        addr += sizeof(Task);
    }
}
