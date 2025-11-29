#ifndef TRAYICONMANAGER_H
#define TRAYICONMANAGER_H

#ifdef HAVE_APPINDICATOR
#include <libayatana-appindicator/app-indicator.h>
#endif

#pragma once

#include <qsystemtrayicon.h>


class TrayIconManager : public QObject
{
    Q_OBJECT

public:
    explicit TrayIconManager(QObject *parent = nullptr);

    void setIcon(const QIcon &icon);
    void setContextMenu(QMenu *menu);
    void show();
    void hide();
    void refreshMenu();

signals:
    void activated(QSystemTrayIcon::ActivationReason reason);

private:
#ifdef HAVE_APPINDICATOR
    AppIndicator *indicator = nullptr;
#endif
    QSystemTrayIcon *tray = nullptr;

    bool useIndicator = false;

    QMenu *m_qmenu = nullptr;
};


#endif // TRAYICONMANAGER_H
