#include <Stream.h>
#include "http_response.h"
#include "string_ext.h"
#include "common.h"

HTTPResponse::HTTPResponse(Stream& stream, bool shouldParseBody) {
    String statusLine = stream.readStringUntil('\r');
    
    String tail;
    String head = bisect(statusLine, " ", tail);
    _statusCode = tail.toInt();

    if (!shouldParseBody) {
        return;
    }

    stream.find("\n\r\n");
    _body = stream.readString();
}
