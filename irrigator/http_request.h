#ifndef __http_request_h
#define __http_request_h

#include <WString.h>
class Stream;

struct HTTPFormField {
    String name;
    String value;
};

class HTTPForm {
public:
    HTTPForm(const String& str);
    ~HTTPForm() {
        if (_fields) {
            delete[] _fields;
        }
    }

    const int fieldCount() const { return _fieldCount; }
    const HTTPFormField& field(int index) const { return _fields[index]; }

private:
    int _fieldCount;
    HTTPFormField* _fields;

};

class HTTPRequest {
public:
    HTTPRequest(Stream& stream);

    const String& method() const { return _method; }
    const String& uri() const { return _uri; }
    const String& query() const { return _query; }
    const String& body() const { return _body; }

private:
    String _method;
    String _uri;
    String _query;
    String _body;
};

#endif // __http_request_h
