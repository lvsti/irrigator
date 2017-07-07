#include <Stream.h>
#include "http_request.h"
#include "string_ext.h"

HTTPForm::HTTPForm(const String& str): _fields(nullptr) {
    _fieldCount = occurrenceCount(str, '&') + 1;

    if (_fieldCount == 0) {
        return;
    }

    _fields = new HTTPFormField[_fieldCount];

    String rest;
    String field = bisect(str, "&", rest);
    for (int i = 0; field.length() > 0; ++i) {
        _fields[i].name = bisect(field, "=", _fields[i].value);
        field = bisect(rest, "&", rest);
    }
}


HTTPRequest::HTTPRequest(Stream& stream) {
    String requestLine = stream.readStringUntil('\r');
    String tail;

    _method = bisect(requestLine, " ", tail);
    String url = bisect(tail, " ", tail);
    _uri = bisect(url, "?", _query);

    // skip the \r\n
    stream.read();
    String headerLine = stream.readStringUntil('\r');

    while (headerLine.length() > 0) {
        HTTPHeaderField header;
        header.name = bisect(headerLine, ": ", header.value);
        _headers.append(header);

        stream.read();
        headerLine = stream.readStringUntil('\r');
    }
    
    _body = stream.readString();
}
