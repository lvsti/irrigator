#ifndef __webservice_h
#define __webservice_h

class HTTPRequest;
class Stream;

extern void handleRequest(const HTTPRequest& request, Stream& responseStream);

#endif // __webservice_h
