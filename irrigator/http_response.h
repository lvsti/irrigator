#ifndef __http_response_h
#define __http_response_h

class Stream;

class HTTPResponse {
public:
    HTTPResponse(Stream& stream, bool shouldParseBody = false);

    const int statusCode() const { return _statusCode; }
    String body() const { return _body; }

private:
    int _statusCode;
    String _body;
};


#endif // __http_response_h
