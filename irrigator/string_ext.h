#ifndef __string_ext_h
#define __string_ext_h

#include <WString.h>

extern int occurrenceCount(const String& src, char needle);
extern String bisect(const String& src, const String& separator, String& tail);

#endif // __string_ext_h
