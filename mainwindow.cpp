#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QProcess>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    trayManager = new TrayIconManager(this);
    trayManager->setIcon(QIcon::fromTheme("computer"));

    // Build tray menu
    QMenu *menu = new QMenu(this);

    menu->addAction("Open Window", this, [this]()
        {
            this->show();
            this->activateWindow();
        });
    menu->addSeparator();
    menu->addAction("Quit", qApp, &QApplication::quit);

    trayManager->setContextMenu(menu);

    // Bind doubleclick to show window
    connect(trayManager, &TrayIconManager::activated, this, [this](QSystemTrayIcon::ActivationReason reason)
            {
                if (reason == QSystemTrayIcon::DoubleClick)
                    {
                        this->show();
                        this->activateWindow();
                    }
            }
    );

    // trayManager->show();

    // Connect button click to runUsbip()
    connect(ui->btnList, &QPushButton::clicked, this, &MainWindow::runUsbip);

    // --- System Tray Setup -----------------------------------
    // tray = new QSystemTrayIcon(this);
    trayManager->setIcon(QIcon::fromTheme("computer"));  // or your own icon

    trayMenu = new QMenu(this);
    trayMenu->addAction("Open Window", this, [this]() {
        this->show();
        this->activateWindow();
    });
    trayMenu->addSeparator();
    trayMenu->addAction("Quit", qApp, &QApplication::quit);

    trayManager->setContextMenu(trayMenu);

    // Double click tray icon to show window
    connect(trayManager, &TrayIconManager::activated, this,
            [this](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::DoubleClick) {
                    this->show();
                    this->activateWindow();
                }
            });

    trayManager->show();
}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Hide window instead of quitting
    if (trayManager)
    {
        this->hide();
        event->ignore();
    }
    else
        QMainWindow::closeEvent(event);
}
