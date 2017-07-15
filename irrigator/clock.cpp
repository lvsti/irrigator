#include "clock.h"

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

static const int kSyncIntervalSeconds = 60 * 60 * 24;

static const char* kNTPServerName = "hu.pool.ntp.org";

static const int kNTPPort = 123;
static const int kNTPPacketSize = 48;
static const int kLocalNTPPort = 2390;

static const unsigned long kUnixEpochStartSeconds = 2208988800UL;

ClockClass Clock;

WiFiUDP udp;
uint8_t packetBuffer[kNTPPacketSize];

// send an NTP request to the time server at the given address
static void sendNTPPacket(IPAddress& address) {
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, kNTPPacketSize);
    // Initialize values needed to form NTP request
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;

    udp.beginPacket(address, kNTPPort);
    udp.write(packetBuffer, kNTPPacketSize);
    udp.endPacket();
}

ClockClass::ClockClass(): 
    _lastSuccessfulSyncTime(DeviceTime::distantPast()), 
    _lastSyncTrialTime(DeviceTime::distantPast()), 
    _startupTime(UnixTime::distantPast()),
    _systemMillisOverflow(0),
    _lastSeenSystemMillis(0) {
}

bool ClockClass::sync() {
    if (!isIsolated() && deviceTime().timeIntervalSince(_lastSuccessfulSyncTime) < TimeInterval::withSeconds(kSyncIntervalSeconds)) {
        return true;
    }

    _lastSyncTrialTime = deviceTime();
    
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    // get a random server from the pool
    IPAddress ntpServerIP;
    if (!WiFi.hostByName(kNTPServerName, ntpServerIP)) {
        return false;
    }

    udp.begin(kLocalNTPPort);

    sendNTPPacket(ntpServerIP);

    // wait to see if a reply is available
    int interval = 10;
    int packetSize = 0;
    while (packetSize == 0 && interval < 1000) {
        delay(interval);
        interval *= 2;
        packetSize = udp.parsePacket();
    }

    DeviceTime syncTime = deviceTime();

    if (packetSize == 0) {
        udp.stop();
        return false;
    }

    // We've received a packet, read the data from it
    int bytesRead = udp.read(packetBuffer, kNTPPacketSize);
    udp.stop();
    
    if (bytesRead != kNTPPacketSize) {
        return false;
    }

    // the timestamp starts at byte 40 of the received packet and is four bytes
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // NTP time
    unsigned long secondsSince1900 = (highWord << 16) | lowWord;
    // unix timestamp
    unsigned long currentTimestamp = secondsSince1900 - kUnixEpochStartSeconds;
    uint32_t startupTimestamp = currentTimestamp - syncTime.timeIntervalSinceReferenceTime().seconds();

    _startupTime = UnixTime(startupTimestamp);
    _lastSuccessfulSyncTime = syncTime;

    return true;
}

DeviceTime ClockClass::deviceTime() {
    unsigned long ms = millis();
    if (ms < _lastSeenSystemMillis) {
        ++_systemMillisOverflow;
    }
    _lastSeenSystemMillis = ms;
    return DeviceTime(ms, _systemMillisOverflow);
}
