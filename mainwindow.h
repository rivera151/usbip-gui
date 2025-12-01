#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "trayiconmanager.h"
#include <qmenu.h>
#include <qmainwindow.h>
#include <qtreewidget.h>

#include "usbipclient.h"
#include "usbiptypes.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void addHost(const QString &hostname);

private:
    Ui::MainWindow *ui;
    TrayIconManager *trayManager;
    void updateToggleActionText();
    QAction *toggleAction;
    UsbipClient *usbipClient;
    QHash<QString, QTreeWidgetItem*> hostItems;
    QTimer *refreshTimer;
    void loadHostsFromDisk();
    void saveHostsToDisk();
    QAction *quitAction;

private Q_SLOTS:
    void runUsbip();
    void onHostDevicesUpdated(const QString &hostname, const QList<UsbipDevice> &devices);
    void onHostError(const QString &hostname, const QString &errorMessage);
    void onPortListUpdated(const QList<UsbipAttachedDevice> &attached);

protected:
    void closeEvent(QCloseEvent *event) override;

};
#endif // MAINWINDOW_H
