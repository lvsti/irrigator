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

String bisect(const String& src, const String& separator, String& tail) {
    if (separator.length() == 0) {
        abort();
    }

    int index = src.indexOf(separator);
    if (index < 0) {
        String head = src;
        tail = "";
        return head;
    }

    String head = src.substring(0, index);
    tail = src.substring(index + separator.length());

    return head;
}

bool base64Decode(const String& src, String& dst) {
    if (src.length() % 4 != 0) {
        return false;
    }

    static const String alphabet = F("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=");
    char buf[src.length()*3 / 4 + 1];

    int j = 0;
    int b[4];

    for (int i = 0; i < src.length(); i += 4) {
        b[0] = alphabet.indexOf(src[i]);
        b[1] = alphabet.indexOf(src[i + 1]);
        b[2] = alphabet.indexOf(src[i + 2]);
        b[3] = alphabet.indexOf(src[i + 3]);
        buf[j++] = (char) ((b[0] << 2) | (b[1] >> 4));
        if (b[2] < 64) {
            buf[j++] = (char) ((b[1] << 4) | (b[2] >> 2));
            if (b[3] < 64)  {
                buf[j++] = (char) ((b[2] << 6) | b[3]);
            }
        }
    }
    buf[j] = 0;
    dst = buf;

    return true;
}
