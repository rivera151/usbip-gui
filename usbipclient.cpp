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

    currentHost = hostname;

    QString program = "usbip";
    QStringList args;
    args << "list" << "-r" << "hostname";

    process->start(program,args);
}

void UsbipClient::onProcessFinished(int exitCode, QProcess::ExitStatus status)
{
    const QString host = currentHost;
    currentHost.clear();

    if (status != QProcess::NormalExit || exitCode != 0)
    {
        QString err = QString::fromLocal8Bit(process->readAllStandardError());
        if (err.trimmed().isEmpty())
        {
            err = tr("usbip list failed with exit code %1").arg(exitCode);
        }
        emit hostError(host,err);
        return;
    }

    QString out = QString::fromLocal8Bit(process->readAllStandardOutput());
    QList<UsbipDevice> devices = parseUsbipListOutput(out);
    emit hostDevicesUpdated(host, devices);
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


