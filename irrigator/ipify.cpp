#include <ESP8266WiFi.h>

#include "ipify.h"
#include "clock.h"
#include "common.h"
#include "http_response.h"

static const TimeInterval kCheckInterval = TimeInterval::withSeconds(60 * 15);


IPifyClass IPify;

IPifyClass::IPifyClass():
    _wasOffline(true),
    _lastCheckCumulativeTime(CumulativeTime::distantPast()) {
}

bool IPifyClass::getExternalIPAddress(String& address) {

    DeviceTime localTime = Clock.deviceTime();
    CumulativeTime dueTime = _lastCheckCumulativeTime + kCheckInterval;
    int32_t seconds = dueTime.timeIntervalSince(Clock.cumulativeTimeFromDeviceTime(localTime)).seconds();

    if (seconds > 0 && !_wasOffline) {
        LOG(F("[ipify] not checking.\n"));
        return false;
    }

    if (WiFi.status() != WL_CONNECTED) {
        LOG(F("[ipify] failed: not connected\n"));
        _wasOffline = true;
        return false;
    }

    IPAddress serverIP;
    if (!WiFi.hostByName("api.ipify.org", serverIP)) {
        LOG(F("[ipify] error: cannot resolve IPify server host\n"));
        return false;
    }

    WiFiClient client;
    client.connect(serverIP, 80);
    int interval = 10;
    while (!client.connected() && interval < 1000) {
        delay(interval);
        interval *= 2;
    }

    if (!client.connected()) {
        LOG(F("[ipify] error: cannot connect to IPify server\n"));
        return false;
    }

    client.println("GET / HTTP/1.1");
    client.println("Host: api.ipify.org");
    client.println();
    client.flush();

    interval = 10;
    while (!client.available() && interval < 1000) {
        delay(interval);
        interval *= 2;
    }

    if (!client.available()) {
        LOG(F("[ipify] error: connection reset\n"));
        return false;
    }

    HTTPResponse response(client, true);
    if (response.statusCode() != 200) {
        LOG(String(F("[ipify] error: server returned ")) + String(response.statusCode()) + F("\n"));
        return false;
    }

    address = response.body();
    client.stop();

    LOG(String(F("[ipify] external IP: [")) + address + F("]\n"));

    _lastCheckCumulativeTime = Clock.cumulativeTimeFromDeviceTime(localTime);
    _wasOffline = false;

    return true;
}