#include "mainwindow.h"
#include "./ui_mainwindow.h"
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
                // Hack to make the new application cursor animation not show most of the time
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
                if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
                {
                    if (this->isVisible())
                    {
                        this->hide();
                    } else
                    {
                        // Hack to make the new application cursor animation not show most of the time
                        this->setWindowState(Qt::WindowNoState);
                        this->show();
                        this->raise();
                        this->activateWindow();
                    }
                    updateToggleActionText();
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
