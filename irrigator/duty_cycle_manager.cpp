#include <EEPROM.h>
#include "eeprom_ext.h"
#include "duty_cycle_manager.h"
#include "irrigator.h"

DutyCycleManagerClass DutyCycleManager;

DutyCycleManagerClass::DutyCycleManagerClass(): _lastCycleTime(0) {
    loadTasks();
}

void DutyCycleManagerClass::updateTask(const Task& task) {
    _tasks[task.valve] = task;
    saveTasks();
}

bool DutyCycleManagerClass::isDue() const {
    UnixTime now;
    if (!getTime(now)) {
        return false;
    }

    return now - _lastCycleTime > 24*60*60;
}

void DutyCycleManagerClass::run() {
    UnixTime now;
    if (!getTime(now)) {
        return;
    }

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

    _lastCycleTime = now;
    put(EEPROM, kEELastDutyCycleTime, _lastCycleTime);
}

void DutyCycleManagerClass::restart() {
    _lastCycleTime = 0;
    put(EEPROM, kEELastDutyCycleTime, _lastCycleTime);
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
