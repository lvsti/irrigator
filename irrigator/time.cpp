#include "time.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "common.h"

const char* kNTPServerName = "hu.pool.ntp.org";

const int kNTPPort = 123;
const int kNTPPacketSize = 48;
const int kLocalNTPPort = 2390;

uint8_t packetBuffer[kNTPPacketSize]; //buffer to hold incoming and outgoing packets

const unsigned long kUnixEpochStartSeconds = 2208988800UL;


// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
UnixTime startupTime;
bool isValidStartupTime = false;

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

bool isNetworkTimeSynced() {
    return isValidStartupTime;
}

bool syncNetworkTime() {
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

    if (packetSize == 0) {
        udp.stop();
        return false;
    }

    // We've received a packet, read the data from it
    udp.read(packetBuffer, kNTPPacketSize); // read the packet into the buffer
    udp.stop();

    // the timestamp starts at byte 40 of the received packet and is four bytes
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // NTP time
    unsigned long secondsSince1900 = (highWord << 16) | lowWord;
    // unix timestamp
    UnixTime now = secondsSince1900 - kUnixEpochStartSeconds;
    unsigned long secondsSinceStartup = millis() / 1000;

    startupTime = now - secondsSinceStartup;
    isValidStartupTime = true;
}

bool getTime(UnixTime& time) {
    if (!isValidStartupTime) {
        return false;
    }

    unsigned long secondsSinceStartup = millis() / 1000;
    time = startupTime + secondsSinceStartup;
    return true;
}
