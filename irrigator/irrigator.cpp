
#include <Arduino.h>
#include <WString.h>
#include "irrigator.h"

IrrigatorClass::IrrigatorClass() : _openValvesMask(0) {
    pinMode(pinForValve(kValveMaster), OUTPUT);

    for (int i = 0; i < kNumOutputValves; ++i) {
        Valve v = outputValves[i];
        pinMode(pinForValve(v), OUTPUT);
        _outputValvesMask |= 1 << v;
    }

    reset();
}

void IrrigatorClass::openValve(Valve valve) {
    LOG(String(F("opening valve ")) + String(valve) + "\n");
    ensureAllOutputValvesAreClosed();
    
    _openValvesMask |= 1 << valve;
    digitalWrite(pinForValve(valve), HIGH);
    delay(kValveOpenTransientTime);
}

void IrrigatorClass::closeValve(Valve valve) {
    digitalWrite(pinForValve(valve), LOW);
    _openValvesMask &= ~(1 << valve);
    delay(kValveCloseTransientTime);
}

void IrrigatorClass::performTask(Task& task) {
    LOG(String(F("starting task for valve ")) + String(task.valve) + ": " + String(task.duration) + "sec\n");
    openValve(task.valve);
    openValve(kValveMaster);

    delay(task.duration * 1000);

    closeValve(kValveMaster);
    closeValve(task.valve);
    LOG(String(F("finishing task for valve ")) + String(task.valve) + "\n");
}

void IrrigatorClass::reset() {
    closeValve(kValveMaster);

    for (int i = 0; i < kNumOutputValves; ++i) {
        Valve v = outputValves[i];
        closeValve(v);
    }
}

void IrrigatorClass::ensureAllOutputValvesAreClosed() {
    if (!(_openValvesMask & _outputValvesMask)) {
        return;
    }

    for (int i = 0; i < kNumOutputValves; ++i) {
        Valve v = outputValves[i];
        if (_openValvesMask & (1 << v)) {
            LOG(String(F("WARNING: valve ")) + String(v) +
                String(F(" was already open when trying to open valve ")) +
                String(v) + "\n");
            closeValve(v);
        }
    }
}

uint8_t IrrigatorClass::pinForValve(Valve valve) {
    return pinD[pinDForValve[valve]];
}

void IrrigatorClass::logOpenMask() {
    LOG(String(F("open valves: ")) + String(_openValvesMask, BIN));
}
