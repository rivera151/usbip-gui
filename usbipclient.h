#ifndef USBIPCLIENT_H
#define USBIPCLIENT_H

#include <QObject>
#include <QProcess>
#include <QHash>
#include "usbiptypes.h"

class UsbipClient : public QObject
{
    Q_OBJECT

public:
    explicit UsbipClient(QObject *parent=nullptr);

    void refreshHost(const QString &hostname);

signals:

    void hostDevicesUpdated(const QString &hostname, const QList<UsbipDevice> &devices);

    void hostError(const QString &hostName, const QString &errorMessage);

private slots:
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    QProcess *process = nullptr;
    QString currentHost;

    QList<UsbipDevice> parseUsbipListOutput(const QString &output) const;

};

#endif // USBIPCLIENT_H
