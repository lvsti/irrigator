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

static const String kBase64Alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

bool base64Decode(const String& src, String& dst) {
    if (src.length() % 4 != 0) {
        return false;
    }

    char buf[src.length()*3 / 4 + 1];

    int j = 0;
    int b[4];

    for (int i = 0; i < src.length(); i += 4) {
        b[0] = kBase64Alphabet.indexOf(src[i]);
        b[1] = kBase64Alphabet.indexOf(src[i + 1]);
        b[2] = kBase64Alphabet.indexOf(src[i + 2]);
        b[3] = kBase64Alphabet.indexOf(src[i + 3]);
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

bool base64Encode(const String& src, String& dst) {
    char buf[src.length()*4 / 3 + 1];

    unsigned int carry = 0;
    int numExtraBits = 0;
    int j = 0;

    for (int i = 0; i < src.length(); ++i) {
        carry |= src[i];
        numExtraBits += 2;

        buf[j++] = kBase64Alphabet[carry >> numExtraBits];
        carry &= (1 << numExtraBits) - 1;

        if (numExtraBits == 6) {
            buf[j++] = kBase64Alphabet[carry];
            carry = 0;
            numExtraBits = 0;
        }

        carry <<= 8;
    }

    if (numExtraBits > 0) {
        buf[j++] = kBase64Alphabet[carry >> (8 + numExtraBits - 6)];
    }

    if (src.length() % 3 != 0) {
        buf[j++] = '=';
    }

    if (src.length() % 3 == 1) {
        buf[j++] = '=';
    }

    buf[j] = 0;
    dst = buf;

    return true;
}

static const char kHexDigits[] = "0123456789ABCDEF";

bool formURLEncode(const String& src, String& dst) {
    String str(src);
    str.replace("%", "%%");

    for (int i = 0; i < str.length(); ++i) {
        int ch = str[i];
        if (!(ch == '%' ||
              ch >= 'A' && ch <= 'Z' || 
              ch >= 'a' && ch <= 'z' || 
              ch >= '0' && ch <= '9' || 
              ch == '-' || ch == '_' || ch == '.' || ch == '~')) {
            char encoded[4] = { '%', 0, 0, 0 };
            encoded[1] = kHexDigits[(ch >> 4) & 0xf];
            encoded[2] = kHexDigits[ch & 0xf];
            str.replace(String((char)ch), encoded);
        }
    }

    dst = str;

    return true;
}

