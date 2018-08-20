#include <ESP8266WiFi.h>

#include "common.h"
#include "http_response.h"
#include "string_ext.h"
#include "thingtweet.h"

static const char kThingTweetAPIKey[] = "*";


bool tweetStatus(const String& status) {
    if (WiFi.status() != WL_CONNECTED) {
        LOG(F("[thingtweet] failed: not connected\n"));
        return false;
    }

    IPAddress serverIP;
    if (!WiFi.hostByName("api.thingspeak.com", serverIP)) {
        LOG(F("[thingtweet] error: cannot resolve server host\n"));
        return false;
    }

    WiFiClient client;
    client.connect(serverIP, 80);
    int interval = 10;
    while (!client.connected() && interval < 1000) {
        delay(interval);
        interval *= 2;
    }

    if (!client.connected()) {
        LOG(F("[thingtweet] error: cannot connect to server\n"));
        return false;
    }

    String formEncoded;
    formURLEncode(status, formEncoded);

    String payload = String("api_key=") + kThingTweetAPIKey + "&status=" + formEncoded;
    String header(F("POST /apps/thingtweet/1/statuses/update HTTP/1.1\r\n"));
    header += F("Host: api.thingspeak.com\r\n");
    header += F("Content-Type: application/x-www-form-urlencoded\r\n");
    header += F("Content-Length: ");
    header += String(payload.length());
    header += F("\r\n\r\n");

    client.print(header);
    client.print(payload);

    interval = 10;
    while (!client.available() && interval < 1000) {
        delay(interval);
        interval *= 2;
    }

    if (!client.available()) {
        LOG(F("[thingtweet] error: connection reset\n"));
        return false;
    }

    HTTPResponse response(client);
    if (response.statusCode() != 200) {
        LOG(String(F("[thingtweet] error: server returned ")) + String(response.statusCode()) + F("\n"));
        return false;
    }

    LOG(String(F("[thingtweet] status tweeted [")) + status + "]\n");

    return true;
}
