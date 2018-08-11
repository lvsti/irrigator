#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "moisture_logger.h"
#include "clock.h"
#include "common.h"
#include "http_response.h"

static const char kIOTPlotterAPIKey[] = "*";
static const char kIOTPlotterFeedID[] = "*";
static const uint32_t kSampleIntervalSeconds = 60 * 15;

MoistureLoggerClass MoistureLogger;

MoistureLoggerClass::MoistureLoggerClass(): 
    _lastSampleCumulativeTime(CumulativeTime::distantPast()) {
    pinMode(pinA[0], INPUT);
}

int MoistureLoggerClass::sample() {
    DeviceTime localTime = Clock.deviceTime();
    CumulativeTime dueTime = _lastSampleCumulativeTime + TimeInterval::withSeconds(kSampleIntervalSeconds);
    int32_t seconds = dueTime.timeIntervalSince(Clock.cumulativeTimeFromDeviceTime(localTime)).seconds();

    if (seconds > 0) {
        LOG(F("[MoistureLogger] not sampling.\n"));
        return -1;
    }

    int value = analogRead(pinA[0]);
    
    if (value < _minValue) {
        _minValue = value;
    }
    if (value > _maxValue) {
        _maxValue = value;
    }

    LOG(String(F("[MoistureLogger] moisture = ")) + String(value) + 
        F(" (min: ") + String(_minValue) + 
        F(", max: ") + String(_maxValue) + 
        F(")\n"));

    _lastSampleCumulativeTime = Clock.cumulativeTimeFromDeviceTime(localTime);

    return value;
}

bool MoistureLoggerClass::submitToIOTPlotter(int value) {
    if (WiFi.status() != WL_CONNECTED) {
        LOG(F("failed: not connected\n"));
        return false;
    }

    IPAddress logServerIP;
    if (!WiFi.hostByName("iotplotter.com", logServerIP)) {
        LOG(F("[MoistureLogger] error: cannot resolve log server host\n"));
        return false;
    }

    WiFiClient client;
    client.connect(logServerIP, 80);
    int interval = 10;
    while (!client.connected() && interval < 1000) {
        delay(interval);
        interval *= 2;
    }

    if (!client.connected()) {
        LOG(F("[MoistureLogger] error: cannot connect to log server\n"));
        return false;
    }

    String payload("{\"data\":{\"moisture\":[{\"value\":" + String(value) + "}]}}");

    String header;
    header += String(F("POST /api/v2/feed/")) + String(kIOTPlotterFeedID) + F("HTTP/1.1\n");
    header += F("Connection: Close\n");
    header += String(F("api-key: ")) + String(kIOTPlotterAPIKey) + F("\n");
    header += F("Content-Type: application/x-www-form-urlencoded\n");
    header += F("Host: iotplotter.com\n");
    header += String(F("Content-Length: ")) + String(payload.length()) + F("\n\n");

    LOG(header);
    LOG(payload + "\n\n----\n\n");

    client.write(header.c_str(), header.length());
    client.write(payload.c_str(), payload.length());
    client.flush();

    interval = 10;
    while (!client.available() && interval < 1000) {
        delay(interval);
        interval *= 2;
    }

    if (!client.available()) {
        LOG(F("[MoistureLogger] error: connection reset\n"));
        return false;
    }

    HTTPResponse response(client);
    if (response.statusCode() != 200) {
        LOG(String(F("[MoistureLogger] error: server returned ")) + String(response.statusCode()) + F("\n"));
        return false;
    }

    client.stop();

    return true;
}