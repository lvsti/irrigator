#include "time.h"

String TimeInterval::toHumanReadableString() const {
    if (*this == TimeInterval::neverInThePast() || *this == TimeInterval::neverInTheFuture()) {
        return "never";
    }

    int s = seconds();
    int hours = abs(s / 3600);
    int minutes = abs((s % 3600) / 60);
    int secs = abs(s % 60);
    String space1 = hours > 0 && minutes > 0 ? " " : "";
    String space2 = (hours > 0 || minutes > 0) && secs > 0 ? " " : "";

    return (hours > 0 ? String(hours) + "h" : String()) + 
           space1 + 
           (minutes > 0 ? String(minutes) + "m" : String()) +
           space2 + 
           (secs > 0 ? String(secs) + "s" : String());
}
