#ifndef __string_ext_h
#define __string_ext_h

#include <WString.h>

extern int occurrenceCount(const String& src, char needle);
extern String bisect(const String& src, const String& separator, String& tail);
extern bool base64Decode(const String& src, String& dst);
extern bool base64Encode(const String& src, String& dst);

#endif // __string_ext_h
