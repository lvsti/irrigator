#include <Stream.h>
#include "webservice.h"
#include "common.h"
#include "duty_cycle_manager.h"
#include "http_request.h"
#include "string_ext.h"
#include "time.h"

static const char kWebserviceCredentials[] = "*:*";

static String renderTaskForm(const DutyCycleManagerClass::Task& task) {
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
    form += F("\"/>sec</p>");
    form += F("<p><input type=\"submit\" value=\"Apply\"/></p></form>");
    return form;
}

static String renderStatusPage() {
    String page = F("HTTP/1.1 200 OK\r\n");
    page += F("Content-Type: text/html\r\n\r\n");
    page += F("<!DOCTYPE HTML><html><title>Irrigator</title><body><h1>Irrigator Status</h1>");
    page += F("<p>Last cycle executed: ");
    page += DutyCycleManager.timeIntervalSinceLastCycle().toHumanReadableString();
    page += F(" ago<br/>Next cycle due: in ");
    page += DutyCycleManager.timeIntervalTillNextCycle().toHumanReadableString();
    page += F("</p>");

    page += F("<p>");
    page += F("<form method=\"post\" action=\"/reset/\">");
    page += F("<input type=\"submit\" value=\"Reset\"/>");
    page += F("</form><br/>");
    page += F("<form method=\"post\" action=\"/reschedule/\">");
    page += F("<input type=\"hidden\" name=\"delay\" value=\"0\"/>");
    page += F("<input type=\"submit\" value=\"Run now\"/>");
    page += F("</form><br/>");
    page += F("<form method=\"post\" action=\"/reschedule/\">");
    page += F("Schedule next cycle: <input type=\"text\" name=\"delay\" value=\"600\"/> seconds from now");
    page += F("<input type=\"submit\" value=\"Schedule\"/>");
    page += F("</form>");
    page += F("</p>");

    for (int i = 0; i < kNumOutputValves; ++i) {
        page += renderTaskForm(DutyCycleManager.task(i));
    }
    page += F("</body></html>");
    return page;
}

static String renderUnauthorized() {
    String page = F("HTTP/1.1 401 Unauthorized\r\n");
    page += F("WWW-Authenticate: Basic realm=\"Irrigator\"\r\n");
    page += F("Content-Type: text/html\r\n\r\n");
    page += F("<html><head><title>401 Unauthorized</title></head><body>");
    page += F("<h1>Unauthorized</h1></body></html>");
    return page;
}

static String renderRedirectToStatusPage() {
    String page = F("HTTP/1.1 303 See Other\r\n");
    page += F("Location: /\r\n\r\n");
    return page;
}

static String renderBadRequest() {
    String page = F("HTTP/1.1 400 Bad Request\r\n");
    page += F("Content-Type: text/html\r\n\r\n");
    page += F("<html><head><title>400 Bad Request</title></head><body>");
    page += F("<h1>Bad Request</h1></body></html>");
    return page;
}

static String renderNotFound() {
    String page = F("HTTP/1.1 404 Not Found\r\n");
    page += F("Content-Type: text/html\r\n\r\n");
    page += F("<html><head><title>404 Not Found</title></head><body>");
    page += F("<h1>Not Found</h1></body></html>");
    return page;
}

static bool isAuthorized(const HTTPRequest& request) {
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

static void handleUpdateValve(const HTTPRequest& request, Stream& responseStream) {
    if (!isAuthorized(request)) {
        responseStream.print(renderUnauthorized());
        return;
    }

    String vstr = request.uri().substring(7, request.uri().length() - 1);
    int v = vstr.toInt();
    bool isInvalidValve = true;

    for (int i = 0; i < kNumOutputValves; ++i) {
        if (v == outputValves[i]) {
            isInvalidValve = false;
            break;
        }
    }

    if (isInvalidValve) {
        LOG(String(F("bad request: ")) + request.method() + " " + request.uri() + "\n");
        responseStream.print(renderBadRequest());
        return;
    }

    // parse valve settings
    HTTPForm form(request.body());
    DutyCycleManagerClass::Task task;
    task.valve = (Valve)v;

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

static void handleResetDutyCycle(const HTTPRequest& request, Stream& responseStream) {
    if (!isAuthorized(request)) {
        responseStream.print(renderUnauthorized());
        return;
    }

    DutyCycleManager.reset();

    responseStream.print(renderRedirectToStatusPage());
}

static void handleRescheduleDutyCycle(const HTTPRequest& request, Stream& responseStream) {
    if (!isAuthorized(request)) {
        responseStream.print(renderUnauthorized());
        return;
    }

    HTTPForm form(request.body());
    int delay = 0;
    for (int i = 0; i < form.fieldCount(); ++i) {
        if (form.field(i).name == F("delay")) {
            delay = form.field(i).value.toInt();
            break;
        }
    }

    if (delay == 0) {
        DutyCycleManager.run();
    }
    else if (delay > 0) {
        DutyCycleManager.schedule(TimeInterval::withSeconds(delay));
    }
    else {
        LOG(String(F("bad request: ")) + request.method() + " " + request.uri() + "\n");
        responseStream.print(renderBadRequest());
        return;
    }

    responseStream.print(renderRedirectToStatusPage());
}

static void handleStatusQuery(const HTTPRequest& request, Stream& responseStream) {
    responseStream.print(renderStatusPage());
}

void handleRequest(const HTTPRequest& request, Stream& responseStream) {
    // route requests
    if (request.method() == "POST" && request.uri().startsWith("/valve/")) {
        handleUpdateValve(request, responseStream);
    }
    else if (request.method() == "POST" && request.uri() == "/reset/") {
        handleResetDutyCycle(request, responseStream);
    }
    else if (request.method() == "POST" && request.uri() == "/reschedule/") {
        handleRescheduleDutyCycle(request, responseStream);
    }
    else if (request.method() == "GET" && request.uri() == "/") {
        handleStatusQuery(request, responseStream);
    }
    else {
        LOG(String(F("not found: ")) + request.method() + " " + request.uri() + "\n");
        responseStream.print(renderNotFound());
    }
}
