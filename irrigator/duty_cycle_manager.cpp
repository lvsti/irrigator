#include "duty_cycle_manager.h"

#include <EEPROM.h>
#include "clock.h"
#include "eeprom_ext.h"
#include "irrigator.h"

static const uint32_t kDutyCycleIntervalSeconds = 60 * 60 * 24;

DutyCycleManagerClass DutyCycleManager;

DutyCycleManagerClass::DutyCycleManagerClass():
    _lastCycleDeviceTime(Clock.deviceTime()), 
    _lastCycleUnixTime(Clock.unixTimeFromDeviceTime(_lastCycleDeviceTime)) {
    loadTasks();
}

void DutyCycleManagerClass::updateTask(const Task& task) {
    _tasks[task.valve] = task;
    saveTasks();
}

bool DutyCycleManagerClass::isDue() {
    DeviceTime now = Clock.deviceTime();
    auto elapsed = now.timeIntervalSince(_lastCycleDeviceTime);

    if (Clock.isIsolated()) {
        return elapsed > TimeInterval::withSeconds(kDutyCycleIntervalSeconds);
    }

    if (_lastCycleUnixTime == UnixTime::distantPast()) {
        _lastCycleUnixTime = Clock.unixTimeFromDeviceTime(_lastCycleDeviceTime);
    }

    elapsed = Clock.unixTimeFromDeviceTime(now).timeIntervalSince(_lastCycleUnixTime); 
    return elapsed > TimeInterval::withSeconds(kDutyCycleIntervalSeconds);
}

void DutyCycleManagerClass::run() {
    DeviceTime now = Clock.deviceTime();

    for (Valve v = 0; v < kNumValves; ++v) {
        const Task& task = _tasks[v];
        if (task.isEnabled) {
            IrrigatorClass::Task t;
            t.valve = task.valve;
            t.duration = task.duration;
            Irrigator.performTask(t);
        } 
        else {
            LOG(String(F("skipping task for valve ")) + String(v) + String(F(": disabled\n")));
        }
    }

    _lastCycleDeviceTime = now;
    _lastCycleUnixTime = Clock.unixTimeFromDeviceTime(_lastCycleDeviceTime);
    put(EEPROM, kEELastDutyCycleDeviceTime, _lastCycleDeviceTime);
    put(EEPROM, kEELastDutyCycleUnixTime, _lastCycleUnixTime.timeIntervalSinceReferenceTime().seconds());
}

void DutyCycleManagerClass::reset() {
    _lastCycleDeviceTime = Clock.deviceTime();
    _lastCycleUnixTime = Clock.unixTimeFromDeviceTime(_lastCycleDeviceTime);
    put(EEPROM, kEELastDutyCycleDeviceTime, _lastCycleDeviceTime);
    put(EEPROM, kEELastDutyCycleUnixTime, _lastCycleUnixTime.timeIntervalSinceReferenceTime().seconds());
}

void DutyCycleManagerClass::loadTasks() {
    int addr = kEETasks;

    for (Valve v = 0; v < kNumValves; ++v) {
        get(EEPROM, addr, _tasks[v]);
        _tasks[v].valve = v;
        addr += sizeof(Task);
    }
}

void DutyCycleManagerClass::saveTasks() {
    int addr = kEETasks;

    for (Valve v = 0; v < kNumValves; ++v) {
        put(EEPROM, addr, _tasks[v]);
        addr += sizeof(Task);
    }
}
