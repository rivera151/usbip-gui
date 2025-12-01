#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "hostsdialog.h"
#include <QTimer>
#include <qevent.h>
#include <qmenu.h>
#include <qprocess.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    trayManager = new TrayIconManager(this);
    trayManager->setIcon(QIcon::fromTheme("computer"));

    // Build tray menu
    QMenu *menu = new QMenu(this);

    toggleAction = new QAction("Show Window", this);
    menu->addAction(toggleAction);

    QAction *hostsAction = new QAction("Hosts...", this);
    menu->addAction(hostsAction);

    connect(hostsAction, &QAction::triggered, this,
        [this]()
        {
            QStringList currentHosts = hostItems.keys();

            HostsDialog dlg(currentHosts, this);
            if (dlg.exec() == QDialog::Accepted)
            {
                QStringList updated = dlg.hosts();

                // Remove hosts no longer present
                for (const QString &h : hostItems.keys())
                {
                    if (!updated.contains(h))
                    {
                        delete hostItems[h];
                        hostItems.remove(h);
                    }
                }

                // Add new hosts
                for (const QString &h : updated)
                {
                    if (!hostItems.contains(h))
                    {
                        addHost(h);
                    }
                }

            }
        }
    );

    menu->addSeparator();
    menu->addAction("Quit", qApp, &QApplication::quit);

    trayManager->setContextMenu(menu);

    updateToggleActionText();

    // Connect button click to runUsbip()
    connect(ui->btnList, &QPushButton::clicked, this, &MainWindow::runUsbip);

    connect(toggleAction, &QAction::triggered, this, [this]()
        {
            if (this->isVisible())
            {
                this->hide();
            } else
            {
                // Hack to make the new application cursor animation not show
                // most of the time
                this->setWindowState(Qt::WindowNoState);
                this->show();
                this->raise();
                this->activateWindow();
            }
            updateToggleActionText();
        });

    // trayManager->setContextMenu(trayMenu);

    // Double click tray icon to show window
    connect(trayManager, &TrayIconManager::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger ||
                    reason == QSystemTrayIcon::DoubleClick)
                {
                    if (this->isVisible())
                    {
                        this->hide();
                    } else
                    {
                        // Hack to make the new application cursor animation
                        // not show most of the time
                        this->setWindowState(Qt::WindowNoState);
                        this->show();
                        this->raise();
                        this->activateWindow();
                    }
                    updateToggleActionText();
                }

            });

    usbipClient = new UsbipClient(this);

    connect(usbipClient, &UsbipClient::hostDevicesUpdated, this,
            &MainWindow::onHostDevicesUpdated);
    connect(usbipClient, &UsbipClient::hostError, this,
            &MainWindow::onHostError);

    // Setup tree widget columns
    ui->treeUsbip->setColumnCount(4);
    ui->treeUsbip->setHeaderLabels(QStringList() << "Name" << "Bus ID"
                                                 << "Vendor" << "Product");

    // Optional for nicer appearance
    ui->treeUsbip->setRootIsDecorated(true);
    ui->treeUsbip->setAlternatingRowColors(true);
    ui->treeUsbip->setSelectionMode(QAbstractItemView::SingleSelection);

    // Auto-refresh timer every 3 seconds
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, [this]()
        {
            for (const QString &host : hostItems.keys())
                usbipClient->refreshHost(host);
        }
    );

    refreshTimer->start(3000);

    // Double click device to toggle attach/detach
    connect(ui->treeUsbip, &QTreeWidget::itemDoubleClicked,
        [this](QTreeWidgetItem *item, int col)
        {
            // Top level host? Ignore...
            if (!item->parent())
                return;

            QString busid = item->text(1);
            QString host = item->parent()->text(0);

            bool attached = item->data(0, Qt::UserRole + 1).toBool();

            if (!attached)
            {
                usbipClient->attachDevice(host, busid);

                // optimistic UI update
                item->setData(0, Qt::UserRole + 1, true);
                // Set green circle
                item->setIcon(0, QIcon::fromTheme("emblem-default"));
            }
            else
            {
                int port = item->data(0, Qt::UserRole + 2).toInt();
                usbipClient->detachPort(port);

                item->setData(0, Qt::UserRole + 1, false);
                item->setIcon(0, QIcon::fromTheme("dialog-error"));
            }

        }
    );

    connect(usbipClient, &UsbipClient::portListUpdated, this,
        [this]()
        {
            usbipClient->refreshPorts();
            // Also refresh hosts
            for (const QString &host : hostItems.keys())
                usbipClient->refreshHost(host);
        }
    );

    trayManager->show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addHost(const QString &hostname)
{
    if (hostItems.contains(hostname))
        return;

    QTreeWidgetItem *hostItem = new QTreeWidgetItem(ui->treeUsbip);
    hostItem->setText(0, hostname);
    hostItem->setIcon(0, QIcon::fromTheme("computer"));
    hostItem->setData(0, Qt::UserRole, hostname);

    hostItems.insert(hostname, hostItem);

    usbipClient->refreshHost(hostname);
}

void MainWindow::runUsbip()
{
    QProcess process;
    process.start("pkexec", {"/usr/sbin/usbip", "list", "--remote=Brutus"});
    process.waitForFinished();

    QString result = process.readAllStandardOutput();
    QString error  = process.readAllStandardError();

    if (!error.isEmpty()) {
        result += "\n---stderr---\n" + error;
    }

    ui->outputBox->setPlainText(result);
}

void MainWindow::onHostDevicesUpdated(const QString &hostname,
                                      const QList<UsbipDevice> &devices)
{
    if (!hostItems.contains(hostname))
        return;

    QTreeWidgetItem *hostItem = hostItems.value(hostname);

    // Clear children
    hostItem->takeChildren();

    for (const UsbipDevice &dev : devices)
    {
        QTreeWidgetItem *devItem = new QTreeWidgetItem();

        devItem->setText(0, dev.description);
        devItem->setText(1, dev.busid);
        devItem->setText(2, dev.vendorId);
        devItem->setText(3, dev.productId);

        // Choose icon dynamically
        QString iconName = "drive-removable-media-usb";

        if (dev.description.contains("Mouse", Qt::CaseInsensitive))
            iconName = "input-mouse";

        if (dev.description.contains("Razer", Qt::CaseInsensitive))
            iconName = "input-mouse";

        if (dev.description.contains("Logitech", Qt::CaseInsensitive))
            iconName = "input-mouse";

        if (dev.description.contains("Keyboard", Qt::CaseInsensitive))
            iconName = "input-keyboard";

        devItem->setIcon(0, QIcon::fromTheme(iconName));

        hostItem->addChild(devItem);

    }

    hostItem->setExpanded(true);

}

void MainWindow::onHostError(const QString &hostname,
                             const QString &errorMessage)
{
    if (!hostItems.contains(hostname))
        return;

    // Fetch the pointer from the key-value pair
    QTreeWidgetItem *hostItem = hostItems.value(hostname);
    hostItem->setText(0, " error");

    // Add a single child entry that contains the error
    hostItem->takeChildren();

    QTreeWidgetItem *errItem = new QTreeWidgetItem();
    errItem->setText(0, "Error: " + errorMessage);
    errItem->setIcon(0, QIcon::fromTheme("dialog-error"));
    hostItem->addChild(errItem);

}

void MainWindow::onPortListUpdated(const QList<UsbipAttachedDevice> &attached)
{
    // Build a quick lookup from busid -> port
    QHash<QString, int> map;
    for (auto &a : attached)
        map[a.busid] = a.port;

    // Iterate all hosts
    for (auto *hostItem : hostItems)
    {
        int childCount = hostItem->childCount();
        for (int i=0; i < childCount; i++)
        {
            QTreeWidgetItem *devItem = hostItem->child(i);
            QString busid = devItem->text(1);

            if (map.contains(busid))
            {
                // Device is attached
                int port = map[busid];
                devItem->setData(0, Qt::UserRole + 1, true);  // attached=true
                devItem->setData(0, Qt::UserRole + 2, port);  // port number
                devItem->setIcon(0, QIcon::fromTheme("emblem-default"));
            }
            else
            {
                devItem->setData(0, Qt::UserRole+1, false);
                devItem->setIcon(0, QIcon::fromTheme("dialog-error"));
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Hide window instead of quitting
    if (trayManager)
    {
        this->hide();
        event->ignore();
        updateToggleActionText();
    } else
        QMainWindow::closeEvent(event);
}

void MainWindow::updateToggleActionText()
{
    if (this->isVisible())
        toggleAction->setText("Hide Window");
    else
        toggleAction->setText("Show Window");

    trayManager->refreshMenu();
}
