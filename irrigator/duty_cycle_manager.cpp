#include "duty_cycle_manager.h"

#include <EEPROM.h>
#include "clock.h"
#include "irrigator.h"

#if DEBUG
static const TimeInterval kDutyCycleInterval = TimeInterval::withSeconds(60);
#else
static const TimeInterval kDutyCycleInterval = TimeInterval::withSeconds(60 * 60 * 24);
#endif

DutyCycleManagerClass DutyCycleManager;

DutyCycleManagerClass::DutyCycleManagerClass():
    _lastCycleCumulativeTime(CumulativeTime::distantPast()),
    _lastCycleUnixTime(UnixTime::distantPast()),
    _isScheduled(false),
    _scheduledCumulativeTime(CumulativeTime::distantPast()),
    _cycleInterval(kDutyCycleInterval) {
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

    EEPROM.get(kEEDutyCycleIntervalSeconds, seconds);
    if (seconds >= 60) {
        _cycleInterval = TimeInterval::withSeconds(seconds);
    }
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
    if (_isScheduled) {
        return _scheduledCumulativeTime.timeIntervalSince(Clock.cumulativeTimeFromDeviceTime(localTime));
    }

    if (Clock.isIsolated()) {
        CumulativeTime dueTime = _lastCycleCumulativeTime + _cycleInterval;
        return dueTime.timeIntervalSince(Clock.cumulativeTimeFromDeviceTime(localTime));
    }

    UnixTime lastCycleUnixTime = _lastCycleUnixTime;
    if (lastCycleUnixTime == UnixTime::distantPast()) {
        lastCycleUnixTime = Clock.unixTimeFromCumulativeTime(_lastCycleCumulativeTime);
    }

    UnixTime dueTime = lastCycleUnixTime + _cycleInterval;
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

    _isScheduled = false;
}

void DutyCycleManagerClass::reset() {
    _lastCycleCumulativeTime = Clock.cumulativeTime();
    uint32_t seconds = _lastCycleCumulativeTime.timeIntervalSinceReferenceTime().seconds();
    EEPROM.put(kEELastDutyCycleCumulativeTimeSeconds, seconds);

    Clock.saveUptime();

    _lastCycleUnixTime = Clock.unixTimeFromCumulativeTime(_lastCycleCumulativeTime);
    seconds = _lastCycleUnixTime.timeIntervalSinceReferenceTime().seconds();
    EEPROM.put(kEELastDutyCycleUnixTimeSeconds, _lastCycleUnixTime);

    _isScheduled = false;
}

void DutyCycleManagerClass::schedule(const TimeInterval& ti) {
    DeviceTime scheduledTime = Clock.deviceTime() + ti;
    _scheduledCumulativeTime = Clock.cumulativeTimeFromDeviceTime(scheduledTime);
    _isScheduled = true;
}

void DutyCycleManagerClass::setCycleInterval(const TimeInterval& ti) {
    if (ti.seconds() < 60) {
        return;
    }

    _cycleInterval = ti;
    EEPROM.put(kEEDutyCycleIntervalSeconds, _cycleInterval.seconds());
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
        LOG(String("loaded task for valve ") + String(outputValves[i]) + ": " +
            TimeInterval::withSeconds(_tasks[i].duration).toHumanReadableString() + "\n");
        addr += sizeof(Task);
    }
}

void DutyCycleManagerClass::saveTasks() {
    int addr = kEETasks;

    for (int i = 0; i < kNumOutputValves; ++i) {
        LOG(String("saving task for valve ") + String(outputValves[i]) + ": " + 
            TimeInterval::withSeconds(_tasks[i].duration).toHumanReadableString());
        EEPROM.put(addr, _tasks[i]);
        addr += sizeof(Task);
    }
}
