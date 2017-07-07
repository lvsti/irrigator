#include <Stream.h>
#include "webservice.h"
#include "common.h"
#include "duty_cycle_manager.h"
#include "http_request.h"
#include "string_ext.h"

static const char kWebserviceCredentials[] = "*:*";

String renderTaskForm(const DutyCycleManagerClass::Task& task) {
    String form = F("<form method=\"post\" action=\"/valve/");
    form += String(task.valve + 1);
    form += F("/\"><h3><input type=\"checkbox\" name=\"is_enabled\"");
    if (task.isEnabled) {
        form += F(" checked=\"checked\"");
    }
    form += F("/> Valve ");
    form += String(task.valve + 1);
    form += F("</h3><p>Description: <input type=\"text\" name=\"description\" maxlength=\"");
    form += String(DutyCycleManagerClass::Task::kDescriptionMaxLength);
    form += F("\" value=\"");
    form += task.description;
    form += F("\"/><br/>");
    form += F("Duration: <input type=\"text\" name=\"duration\" value=\"");
    form += task.duration;
    form += F("\"/>ms</p>");
    form += F("<p><input type=\"submit\" value=\"Apply\"/></p></form>");
    return form;
}

String renderStatusPage() {
    String page = F("HTTP/1.1 200 OK\r\n");
    page += F("Content-Type: text/html\r\n\r\n");
    page += F("<!DOCTYPE HTML><html><title>Irrigator</title><body><h1>Irrigator Status</h1>");
    for (Valve v = 0; v < kNumValves; ++v) {
        page += renderTaskForm(DutyCycleManager.task(v));
    }
    page += F("</p></body></html>");
    return page;
}

String renderUnauthorized() {
    String page = F("HTTP/1.1 401 Unauthorized\r\n");
    page += F("WWW-Authenticate: Basic realm=\"Irrigator\"\r\n");
    page += F("Content-Type: text/html\r\n\r\n");
    page += F("<html><head><title>401 Unauthorized</title></head><body>");
    page += F("<h1>Unauthorized</h1></body></html>");
    return page;
}

String renderRedirectToStatusPage() {
    String page = F("HTTP/1.1 303 See Other\r\n");
    page += F("Location: /\r\n\r\n");
    return page;
}

String renderBadRequest() {
    String page = F("HTTP/1.1 400 Bad Request\r\n");
    page += F("Content-Type: text/html\r\n\r\n");
    page += F("<html><head><title>400 Bad Request</title></head><body>");
    page += F("<h1>Bad Request</h1></body></html>");
    return page;
}

String renderNotFound() {
    String page = F("HTTP/1.1 404 Not Found\r\n");
    page += F("Content-Type: text/html\r\n\r\n");
    page += F("<html><head><title>404 Not Found</title></head><body>");
    page += F("<h1>Not Found</h1></body></html>");
    return page;
}

bool isAuthorized(const HTTPRequest& request) {
    auto it = request.headers().iterator();
    HTTPHeaderField* field = it.next();
    while (field) {
        if (field->name == F("Authorization") && field->value.startsWith("Basic ")) {
            String credentialsB64;
            String credentials;
            bisect(field->value, " ", credentialsB64);
            if (base64Decode(credentialsB64, credentials) && credentials == kWebserviceCredentials) {
                return true;
            }
        }
        field = it.next();
    }

    return false;
}

void handleUpdateValve(const HTTPRequest& request, Stream& responseStream) {
    if (!isAuthorized(request)) {
        responseStream.print(renderUnauthorized());
        return;
    }

    String vstr = request.uri().substring(7, request.uri().length() - 1);
    int v = vstr.toInt();
    if (v == 0 || v > kNumValves) {
        LOG(String(F("bad request: ")) + request.method() + " " + request.uri() + "\n");
        responseStream.print(renderBadRequest());
        return;
    }

    --v;

    // parse valve settings
    HTTPForm form(request.body());
    DutyCycleManagerClass::Task task;
    task.valve = v;

    for (int i = 0; i < form.fieldCount(); ++i) {
        if (form.field(i).name == F("description")) {
            form.field(i).value.toCharArray(task.description, DutyCycleManagerClass::Task::kDescriptionMaxLength);
        } 
        else if (form.field(i).name == F("duration")) {
            task.duration = form.field(i).value.toInt();
        } 
        else if (form.field(i).name == F("is_enabled")) {
            task.isEnabled = true;
        }
    }
    
    // apply settings
    DutyCycleManager.updateTask(task);

    responseStream.print(renderRedirectToStatusPage());
}

void handleRestartDutyCycle(const HTTPRequest& request, Stream& responseStream) {
    if (!isAuthorized(request)) {
        responseStream.print(renderUnauthorized());
        return;
    }

    DutyCycleManager.restart();

    responseStream.print(renderRedirectToStatusPage());
}

void handleStatusQuery(const HTTPRequest& request, Stream& responseStream) {
    responseStream.print(renderStatusPage());
}

void handleRequest(const HTTPRequest& request, Stream& responseStream) {
    // route requests
    if (request.method() == "POST" && request.uri().startsWith("/valve/")) {
        handleUpdateValve(request, responseStream);
    }
    else if (request.method() == "POST" && request.uri() == "/restart/") {
        handleRestartDutyCycle(request, responseStream);
    }
    else if (request.method() == "GET" && request.uri() == "/") {
        handleStatusQuery(request, responseStream);
    }
    else {
        LOG(String(F("not found: ")) + request.method() + " " + request.uri() + "\n");
        responseStream.print(renderNotFound());
    }
}
