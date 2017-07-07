#include "string_ext.h"
#include "common.h"

int occurrenceCount(const String& src, char needle) {
    int startOffset = 0;
    int count = 0;

    for (int i = 0; i < src.length(); ++i) {
        if (src[i] == needle) {
            ++count;
        }
    }

    return count;
}

String bisect(const String& src, char separator, String& tail) {
    int index = src.indexOf(separator);
    if (index < 0) {
        String head = src;
        tail = "";
        return head;
    }

    String head = src.substring(0, index);
    tail = src.substring(index + 1);

    return head;
}
