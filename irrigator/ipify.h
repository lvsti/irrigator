#ifndef __ipify_h
#define __ipify_h

#include "time.h"

class IPifyClass {
public:
    IPifyClass();
    bool getExternalIPAddress(String& address);

private:
    CumulativeTime _lastCheckCumulativeTime;
    bool _wasOffline;
};

extern IPifyClass IPify;

#endif // __ipify_h
