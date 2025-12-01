#include "usbipclient.h"
#include <QRegularExpression>
#include <qtextstream.h>


UsbipClient::UsbipClient(QObject *parent) : QObject(parent)
{
    process = new QProcess(this);

    connect(process, &QProcess::finished, this, &UsbipClient::onProcessFinished);
}

void UsbipClient::refreshHost(const QString &hostname)
{
    if (process->state() == QProcess::NotRunning)
        return;

    currentOp = Operation::List;

    currentHost = hostname;

    QString program = "usbip";
    QStringList args;
    args << "list" << "-r" << "hostname";

    process->start(program,args);
}

void UsbipClient::attachDevice(const QString &hostname, const QString &busid)
{
    if (process->state() != QProcess::NotRunning)
        return;

    currentOp = Operation::Attach;
    currentHost = hostname;

    QString program = "pkexec";

    QStringList args;
    args << "usbip" << "attach" << "-r" << hostname << "-b" << busid;
    process->start(program, args);
}

void UsbipClient::detachPort(const int port)
{
    if (process->state() != QProcess::NotRunning)
        return;

    currentOp = Operation::Detach;

    QString program = "pkexec";
    QStringList args;
    args << "usbip" << "detach" << "-p" << QString::number(port);

    process->start(program, args);
}

void UsbipClient::refreshPorts()
{
    if (process->state() != QProcess::NotRunning)
        return;

    currentOp = Operation::List;
    currentHost.clear();

    QString program = "usbip";
    QStringList args;
    args << "port";
    process->start(program, args);
}

void UsbipClient::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    // Read standard out and standard error
    QString stdoutData = QString::fromLocal8Bit(process->readAllStandardOutput());
    QString stdError = QString::fromLocal8Bit(process->readAllStandardError());
    const QString host = currentHost;

    Operation op = currentOp;
    currentOp = Operation::None;
    currentHost.clear();

    if (status != QProcess::NormalExit || exitCode != 0)
    {
        Q_EMIT hostError(host, stdError.trimmed());
        return;
    }

    if (op == Operation::List)
    {
        auto devices = parseUsbipListOutput(stdoutData);
        Q_EMIT hostDevicesUpdated(host, devices);
    }
    else if (op == Operation::Attach)
    {
        Q_EMIT hostError(host, "Device attached");  // placeholder
        // Later: emit signal with attached port
    }
    else if (op == Operation::Detach)
    {
        Q_EMIT hostError(host, "Device detached");
        // Later: emit clean result
    }
    else // port listing
    {
        auto ports = parseUsbipPortOutput(stdoutData);
        Q_EMIT portListUpdated(ports);
    }
}

QList<UsbipDevice> UsbipClient::parseUsbipListOutput(const QString &output) const
{
    QList<UsbipDevice> result;

    QTextStream stream(output.toUtf8());
    QString line;
    UsbipDevice current;
    bool inDevice = false;

    // Regex for lines like:
    //  - 1-1: Logitech USB Receiver
    //  - 1-1.2: Some USB Device
    QRegularExpression reDeviceLine(
        R"(^\s*-\s*([0-9]+-[0-9\.]+)\s*[: ]\s*(.*)$)"
        );

    // Regex for lines like:
    //   : (046d:c534) Logitech, Inc. : USB Receiver
    QRegularExpression reIdLine(
        R"(\(([0-9a-fA-F]{4}):([0-9a-fA-F]{4})\))"
        );

    while (stream.readLineInto(&line))
    {
        line =  line.trimmed();
        if (line.isEmpty())
            continue;

        QRegularExpressionMatch m = reDeviceLine.match(line);
        if (m.hasMatch())
        {
            // Strat new device
            if (inDevice)
                result.append(current);

            current = UsbipDevice{};
            inDevice = true;

            current.busid = m.captured(1).trimmed();
            current.productId = m.captured(2).trimmed();

            continue;
        }

        if (!inDevice)
            continue;

        // Look for vendor/product ID in subsequent lines
        QRegularExpressionMatch idMatch = reIdLine.match(line);
        if (idMatch.hasMatch())
        {
            current.vendorId = idMatch.captured(1).toLower();
            current.productId = idMatch.captured(2).toLower();
        }

        // Try to enrich description from other lines if they contain useful text
        // e.g. "Logitech, Inc. : USB Receiver"
        if (!line.startsWith(":"))
        {
            if (!current.description.isEmpty())
                current.description += " ";
            current.description += line;
        }


    }

    if (inDevice) {
        result.append(current);
    }

    return result;
}

QList<UsbipAttachedDevice> UsbipClient::parseUsbipPortOutput(const QString &output) const
{
    QList<UsbipAttachedDevice> result;

    QTextStream stream(output.toUtf8());
    QString line;
    QRegularExpression rePort(R"(Port\s+(\d+):\s+<([^>]+)>)");
    QRegularExpression reBusid(R"(\s*busid\s+([0-9A-Za-z\.-]+))");

    int currentPort = -1;

    while (stream.readLineInto(&line))
    {
        line = line.trimmed();

        QRegularExpressionMatch mPort = rePort.match(line);

        if (mPort.hasMatch())
        {
            QString status = mPort.captured(2);
            currentPort = mPort.captured(1).toInt();

            // Only ports "in use" have devices
            if (status.contains("in use", Qt::CaseInsensitive))
                currentPort = -1;

            continue;
        }

        if (currentPort >= 0)
        {
            QRegularExpressionMatch mBus = reBusid.match(line);
            if (mBus.hasMatch())
            {
                UsbipAttachedDevice d;
                d.port = currentPort;
                d.busid = mBus.captured(1);
                result.append(d);

                currentPort = -1;
            }
        }
    }

    return result;
}

