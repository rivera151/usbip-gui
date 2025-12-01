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

    void attachDevice(const QString &hostname, const QString &busid);

    void detachPort(const int port);

    void refreshPorts();

Q_SIGNALS:
    void hostDevicesUpdated(const QString &hostname,
                            const QList<UsbipDevice> &devices);

    void hostError(const QString &hostname,
                   const QString &errorMessage);

    void portListUpdated(const QList<UsbipAttachedDevice> &attached);

private Q_SLOTS:
    void onProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    QProcess *process = nullptr;
    QString currentHost;

    QList<UsbipDevice> parseUsbipListOutput(const QString &output) const;
    QList<UsbipAttachedDevice> parseUsbipPortOutput(const QString &output)
        const;

    enum class Operation
    {
        None,
        List,
        Attach,
        Detach
    };

    Operation currentOp = Operation::None;
};

#endif // USBIPCLIENT_H
