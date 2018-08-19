#ifndef __ddns_h
#define __ddns_h

#include "time.h"

class DDNSClass {
public:
    DDNSClass();
    bool getExternalIPAddress(String& address);
    bool updateDDNS();

private:
    CumulativeTime _lastUpdateCumulativeTime;
    bool _wasOffline;
};

extern DDNSClass DDNS;

#endif // __ddns_h
