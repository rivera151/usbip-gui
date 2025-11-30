#ifndef USBIPTYPES_H
#define USBIPTYPES_H

#include <qstring.h>

struct UsbipDevice
{
    QString busid;         // e.g. "1-1", "2-3.4"
    QString description;   // human-readable description
    QString vendorId;      // "046d"
    QString productId;     // "c534"
    QString vendorName;    // optional, if parseable
    QString productName;   // optional, if parseable

    bool    attached = false;   // we'll manage this later (attach/detach)
    int     port = -1;          // usbip port if attached (from usbip attach output)
};

#endif // USBIPTYPES_H
