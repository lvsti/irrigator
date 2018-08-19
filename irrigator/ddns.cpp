#include <ESP8266WiFi.h>

#include "ddns.h"
#include "clock.h"
#include "common.h"
#include "http_response.h"
#include "string_ext.h"

static const char kNoIPUsername[] = "*";
static const char kNoIPPassword[] = "*";
static const char kNoIPHostname[] = "*";

static const TimeInterval kUpdateInterval = TimeInterval::withSeconds(60 * 15);


DDNSClass DDNS;

DDNSClass::DDNSClass():
    _wasOffline(true),
    _lastUpdateCumulativeTime(CumulativeTime::distantPast()) {
}

bool DDNSClass::getExternalIPAddress(String& address) {
    if (WiFi.status() != WL_CONNECTED) {
        LOG(F("[ipify] failed: not connected\n"));
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

    client.println(F("GET / HTTP/1.1"));
    client.println(F("Host: api.ipify.org"));
    client.println();

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

    return true;
}

bool DDNSClass::updateDDNS() {
    DeviceTime localTime = Clock.deviceTime();
    CumulativeTime dueTime = _lastUpdateCumulativeTime + kUpdateInterval;
    int32_t seconds = dueTime.timeIntervalSince(Clock.cumulativeTimeFromDeviceTime(localTime)).seconds();

    if (seconds > 0 && !_wasOffline) {
        LOG(F("[noip] not updating.\n"));
        return false;
    }

    if (WiFi.status() != WL_CONNECTED) {
        LOG(F("[noip] failed: not connected\n"));
        _wasOffline = true;
        return false;
    }

    IPAddress serverIP;
    if (!WiFi.hostByName("dynupdate.no-ip.com", serverIP)) {
        LOG(F("[noip] error: cannot resolve No-IP server host\n"));
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
        LOG(F("[noip] error: cannot connect to No-IP server\n"));
        return false;
    }

    String credentials;
    base64Encode(String(kNoIPUsername) + ":" + kNoIPPassword, credentials);

    String request(F("GET /nic/update?hostname="));
    request += kNoIPHostname;
    request += F(" HTTP/1.1\r\n");
    request += F("Host: dynupdate.no-ip.com\r\n");
    request += F("User-Agent: Irrigator/1.0 maintainer@domain.com\r\n");
    request += F("Authorization: Basic ");
    request += credentials;
    request += F("\r\n");
    request += F("\r\n");
    client.print(request);

    interval = 10;
    while (!client.available() && interval < 1000) {
        delay(interval);
        interval *= 2;
    }

    if (!client.available()) {
        LOG(F("[noip] error: connection reset\n"));
        return false;
    }

    HTTPResponse response(client, true);
    if (response.statusCode() != 200) {
        LOG(String(F("[noip] error: server returned ")) + String(response.statusCode()) + F("\n"));
        return false;
    }

    client.stop();

    if (response.body().indexOf("nochg") == -1 && response.body().indexOf("good") == -1) {
        LOG(String(F("[noip] error: DDNS update failed [")) + response.body() + F("]\n"));
        return false;
    }

    LOG(F("[noip] DDNS updated successfully.\n"));
    _lastUpdateCumulativeTime = Clock.cumulativeTimeFromDeviceTime(localTime);
    _wasOffline = false;

    return true;
}
