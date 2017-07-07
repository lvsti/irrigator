
#include <Arduino.h>
#include <WString.h>
#include "irrigator.h"

IrrigatorClass::IrrigatorClass() : _openValvesMask(0) {
    for (Valve v = 0; v < kNumValves; ++v) {
        pinMode(pinForValve(v), OUTPUT);
        closeValve(v);
    }
}

void IrrigatorClass::openValve(Valve valve) {
    LOG(String(F("opening valve ")) + String(valve) + "\n");
    prepareValveForOpening(valve);
    _openValvesMask |= 1 << valve;
    digitalWrite(pinForValve(valve), HIGH);
}

void IrrigatorClass::closeValve(Valve valve) {
    digitalWrite(pinForValve(valve), LOW);
    _openValvesMask &= ~(1 << valve);
}

void IrrigatorClass::performTask(Task& task) {
    LOG(String(F("starting task for valve ")) + String(task.valve) + ": " + String(task.duration) + "ms\n");
    openValve(task.valve);
    delay(kValveOpenTransientTime);

    delay(task.duration);

    closeValve(task.valve);
    delay(kValveCloseTransientTime);
    LOG(String(F("finishing task for valve ")) + String(task.valve) + "\n");
}

void IrrigatorClass::reset() {
    for (Valve v = 0; v < kNumValves; ++v) {
        closeValve(v);
    }
}

void IrrigatorClass::prepareValveForOpening(Valve valve) {
    if (!_openValvesMask) {
        return;
    }

    for (Valve v = 0; v < kNumValves; ++v) {
        if (_openValvesMask & (1 << v)) {
            LOG(String(F("WARNING: valve ")) + String(v) +
                String(F(" was already open when trying to open valve ")) +
                String(valve) + "\n");
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
