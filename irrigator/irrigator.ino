#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <Ticker.h>
#include "clock.h"
#include "common.h"
#include "duty_cycle_manager.h"
#include "http_request.h"
#include "moisture_logger.h"
#include "webservice.h"

static const TimeInterval kConnectionTimeout = TimeInterval::withSeconds(10);
static const TimeInterval kWatchdogTimerInterval = TimeInterval::withSeconds(30);

Ticker watchdog;

void watchdogHandler() {
    LOG(F("[main] watchdog alert: loop is stuck, resetting board.\n"));
    void (*resetBoard)(void) = 0;
    resetBoard();
}

bool ensureWifiConnection() {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    const char ssid[] = "*";
    const char password[] = "*";
    LOG(String(F("\n\nConnecting to ")) + ssid);

    WiFi.begin(ssid, password);

    DeviceTime startTime = Clock.deviceTime();

    while (WiFi.status() != WL_CONNECTED) {
        if (Clock.deviceTime().timeIntervalSince(startTime) > kConnectionTimeout) {
            LOG(F("\nCouldn't connect to WiFi\n"));
            return false;
        }
        delay(500);
        LOG(".");
    }

    LOG(F("\nWiFi connected\n"));
}


WiFiServer server(80);


void serve(WiFiClient& client) {
    while (!client.available()) {
        delay(10);
    }

    HTTPRequest request(client);
    client.flush();

    handleRequest(request, client);
}

void setup() {
    Serial.begin(115200);
    delay(10);

    LOG(String(F("[main] EEPROM size: ")) + String(kEESize) + "\n");

    EEPROM.begin(kEESize);
    uint16_t firmwareVersion = -1;
    EEPROM.get(kEEFirmwareVersion, firmwareVersion);

    if (firmwareVersion != kFirmwareVersion) {
        // reset EEPROM
        LOG(F("[main] firmware version changed, resetting EEPROM\n"));
        uint8_t* ptr = EEPROM.getDataPtr();
        memset(ptr, 0, kEESize);
        EEPROM.put(kEEFirmwareVersion, kFirmwareVersion);
        EEPROM.commit();
    }

    uint8_t* ptr = EEPROM.getDataPtr();
    for (int i = 0; i < kEESize; ++i, ++ptr) {
        LOG(String(*ptr, 16) + " ");
        if (i % 16 == 15) {
            LOG("\n");
        }
    }
    LOG("\n");

    Clock.loadUptime();
    DutyCycleManager.loadState();

    ensureWifiConnection();

    server.begin();

    watchdog.once(kWatchdogTimerInterval.seconds(), watchdogHandler);
}

void loop() {
    unsigned long startTime = millis();
    LOG(F("loop starts\n"));
    ensureWifiConnection();

    // Check if a client has connected
    WiFiClient client = server.available();
    if (client) {
        serve(client);
    }

    Clock.sync();

    if (DutyCycleManager.isDue()) {
        watchdog.detach();
        DutyCycleManager.run();
        watchdog.once(kWatchdogTimerInterval.seconds(), watchdogHandler);
    }

    int moisture = MoistureLogger.sample();
    if (moisture > 0) {
        MoistureLogger.submitToThingspeak(moisture);
    }

    if (!EEPROM.commit()) {
        LOG(F("[main] EEPROM commit failed\n"));
    }

    watchdog.once(kWatchdogTimerInterval.seconds(), watchdogHandler);

    unsigned long endTime = millis();
    LOG(String(F("loop ends (")) + String(endTime - startTime) + "ms)\n");
    delay(200);
}
