#ifndef __time_h
#define __time_h

typedef unsigned long UnixTime;

extern bool isNetworkTimeSynced();
extern bool syncNetworkTime();
extern bool getTime(UnixTime& time);

#endif __time_h
