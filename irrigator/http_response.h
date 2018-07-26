#ifndef __http_response_h
#define __http_response_h

class Stream;

class HTTPResponse {
public:
    HTTPResponse(Stream& stream);

    const int statusCode() const { return _statusCode; }

private:
    int _statusCode;
};


#endif // __http_response_h
