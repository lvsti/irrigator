#include <Stream.h>
#include "http_response.h"
#include "string_ext.h"
#include "common.h"

HTTPResponse::HTTPResponse(Stream& stream, bool shouldParseBody) {
    String statusLine = stream.readStringUntil('\r');
    // skip the \n
    stream.read();
    
    String tail;
    String head = bisect(statusLine, " ", tail);
    _statusCode = tail.toInt();

    if (!shouldParseBody) {
        return;
    }
    
    String line;
    do {
        line = stream.readStringUntil('\r');
        stream.read();
    }
    while (line.length() > 0);

    _body = stream.readString();
}
