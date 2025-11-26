#ifndef TRAYICONMANAGER_H
#define TRAYICONMANAGER_H

#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>

class TrayIconManager : public QObject
{
    Q_OBJECT

public:
    explicit TrayIconManager(QObject *parent = nullptr);

    void setIcon(const QIcon &icon);
    void setContextMenu(QMenu *menu);
    void show();
    void hide();

signals:
    void activated(QSystemTrayIcon::ActivationReason reason);

private:
    QSystemTrayIcon *tray;
};


#endif // TRAYICONMANAGER_H
