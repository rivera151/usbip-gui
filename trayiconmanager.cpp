#include "trayiconmanager.h"

TrayIconManager::TrayIconManager(QObject *parent) : QObject(parent)
{
    tray = new QSystemTrayIcon(this);

    connect(tray, &QSystemTrayIcon::activated, this, &TrayIconManager::activated);
}

void TrayIconManager::setIcon(const QIcon &icon)
{
    tray->setIcon(icon);
}

void TrayIconManager::setContextMenu(QMenu *menu)
{
    tray->setContextMenu(menu);
}

void TrayIconManager::show()
{
    if (QSystemTrayIcon::isSystemTrayAvailable())
        tray->show();
    else
        qWarning("System tray not available — icon will not be visible.");
}

void TrayIconManager::hide()
{
    tray->hide();
}

