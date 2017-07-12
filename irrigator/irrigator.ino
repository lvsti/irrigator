#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "clock.h"
#include "common.h"
#include "duty_cycle_manager.h"
#include "eeprom_ext.h"
#include "http_request.h"
#include "webservice.h"

void connectToWiFi() {
    const char ssid[] = "*";
    const char password[] = "*";
    LOG(String(F("\n\nConnecting to ")) + ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
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

    EEPROM.begin(kEESize);
    unsigned int firmwareVersion = -1;
    get(EEPROM, kEEFirmwareVersion, firmwareVersion);

    if (firmwareVersion != kFirmwareVersion) {
        // reset EEPROM
        uint8_t* ptr = EEPROM.getDataPtr();
        for (int i = 0; i < kEESize; ++i, ++ptr) {
            *ptr = 0;
        }
        put(EEPROM, kEEFirmwareVersion, firmwareVersion);
        EEPROM.commit();
    }

    connectToWiFi();

    server.begin();
}

void loop() {
    // Check if a client has connected
    WiFiClient client = server.available();
    if (client) {
        serve(client);
    }

    Clock.sync();

    if (DutyCycleManager.isDue()) {
        DutyCycleManager.run();
    }

    EEPROM.commit();
    delay(1000);
}
