#include <Stream.h>
#include "http_response.h"
#include "string_ext.h"

HTTPResponse::HTTPResponse(Stream& stream) {
    String statusLine = stream.readStringUntil('\r');
    // skip the \n
    stream.read();
    
    String tail;
    _statusCode = bisect(statusLine, " ", tail).toInt();
}
