#include "clock.h"

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "common.h"

#if DEBUG
static const TimeInterval kSyncRetryInterval = TimeInterval::withSeconds(10);
static const TimeInterval kSyncInterval = TimeInterval::withSeconds(60);
static const TimeInterval kUptimeSaveInterval = TimeInterval::withSeconds(30);
#else
static const TimeInterval kSyncRetryInterval = TimeInterval::withSeconds(60);
static const TimeInterval kSyncInterval = TimeInterval::withSeconds(60 * 60 * 24);
static const TimeInterval kUptimeSaveInterval = TimeInterval::withSeconds(60 * 60);
#endif

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
    _lastUptimeSaveTime(DeviceTime::distantPast()),
    _startupTime(UnixTime::distantPast()),
    _firstStartupTime(UnixTime::distantPast()),
    _previousUptime(TimeInterval::withSeconds(0)),
    _systemMillisOverflow(0),
    _lastSeenSystemMillis(0) {
    
    _lastUptimeSaveTime = deviceTime();
}

bool ClockClass::sync() {
    DeviceTime localTime = deviceTime();
    LOG(String(F("[Clock] Device time is: ")) + 
        localTime.timeIntervalSinceReferenceTime().toHumanReadableString() + 
        " (" + String(localTime.timeIntervalSinceReferenceTime().seconds()) + ")\n");
    CumulativeTime ct = cumulativeTime();
    LOG(String(F("[Clock] Cumulative uptime is: ")) + 
        ct.timeIntervalSinceReferenceTime().toHumanReadableString() + 
        " (" + String(ct.timeIntervalSinceReferenceTime().seconds()) + ")\n");

    if (_lastUptimeSaveTime == DeviceTime::distantPast() ||
        localTime.timeIntervalSince(_lastUptimeSaveTime) > kUptimeSaveInterval) {
        saveUptime();
    }

    if (isIsolated() && 
        (_lastSyncTrialTime != DeviceTime::distantPast() && 
         localTime.timeIntervalSince(_lastSyncTrialTime) < kSyncRetryInterval)) {
        LOG(F("[Clock] Not syncing (isolated): timeout hasn't passed since last failed trial\n"));
        return false;
    }

    if (!isIsolated() && localTime.timeIntervalSince(_lastSuccessfulSyncTime) < kSyncInterval) {
        LOG(F("[Clock] Not syncing (already synced): timeout hasn't passed since last successful sync\n"));
        return true;
    }

    _lastSyncTrialTime = localTime;

    LOG(F("[Clock] Syncing network time..."));
    
    if (WiFi.status() != WL_CONNECTED) {
        LOG(F("failed: not connected\n"));
        return false;
    }

    // get a random server from the pool
    IPAddress ntpServerIP;
    if (!WiFi.hostByName(kNTPServerName, ntpServerIP)) {
        LOG(F("failed: cannot resolve NTP host\n"));
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
        LOG(F("failed: empty response\n"));
        return false;
    }

    // We've received a packet, read the data from it
    int bytesRead = udp.read(packetBuffer, kNTPPacketSize);
    udp.stop();
    
    if (bytesRead != kNTPPacketSize) {
        LOG(F("failed: unexpected NTP response\n"));
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
    uint32_t firstStartupTimestamp = currentTimestamp - cumulativeTimeFromDeviceTime(syncTime).timeIntervalSinceReferenceTime().seconds();

    _startupTime = UnixTime(startupTimestamp);
    _firstStartupTime = UnixTime(firstStartupTimestamp);

    _lastSuccessfulSyncTime = syncTime;
    
    LOG(String(currentTimestamp) + "\n");

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

void ClockClass::loadUptime() {
    uint32_t uptimeSeconds = 0;
    EEPROM.get(kEEPreviousUptimeSeconds, uptimeSeconds);
    _previousUptime = TimeInterval::withSeconds(uptimeSeconds);
    LOG(String(F("[Clock] Loaded previous uptime: ")) + 
        _previousUptime.toHumanReadableString() + 
        " (" +
        String(uptimeSeconds) +
        ")\n");
}

void ClockClass::saveUptime() {
    DeviceTime localTime = deviceTime();
    uint32_t uptimeSeconds = _previousUptime.seconds() + localTime.timeIntervalSinceReferenceTime().seconds();
    EEPROM.put(kEEPreviousUptimeSeconds, uptimeSeconds);
    _lastUptimeSaveTime = localTime;
    LOG(String(F("[Clock] Saved current uptime (")) + 
        TimeInterval::withSeconds(uptimeSeconds).toHumanReadableString() +
        ")\n");
}
