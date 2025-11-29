#include "trayiconmanager.h"
#include "gtkmenushim.h"
#include <qprocess.h>


TrayIconManager::TrayIconManager(QObject *parent) : QObject(parent)
{
    QString desktop = QProcessEnvironment::systemEnvironment().value("XDG_CURRENT_DESKTOP");

    // GNOME + indicator library available?
#ifdef HAVE_APPINDICATOR
    if (desktop.contains("GNOME", Qt::CaseInsensitive)) {
        useIndicator = true;

        indicator = app_indicator_new_with_path(
            "usbip-gui-indicator",
            "computer",                   // fallback icon name
            APP_INDICATOR_CATEGORY_APPLICATION_STATUS,
            "/usr/share/icons"
            );

        app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
        return;     // Finished initializing Indicator backend
    }
#endif

    // Fallback to QSystemTrayIcon
    tray = new QSystemTrayIcon(this);
    connect(tray, &QSystemTrayIcon::activated, this, &TrayIconManager::activated);
}

void TrayIconManager::setIcon(const QIcon &icon)
{
#ifdef HAVE_APPINDICATOR
    if (useIndicator) {
        // AppIndicator requires icon file paths, not QIcons
        QString path = icon.name();
        app_indicator_set_icon(indicator, path.toUtf8().constData());
        return;
    }
#endif
    tray->setIcon(icon);
}

void TrayIconManager::setContextMenu(QMenu *menu)
{
    m_qmenu = menu;

#ifdef HAVE_APPINDICATOR
    if (useIndicator) {
        GtkMenuShim shim(menu);
        GtkWidget *gtkMenu = shim.gtkMenu();
        app_indicator_set_menu(indicator, GTK_MENU(gtkMenu));
        return;
    }
#endif
    tray->setContextMenu(menu);
}

void TrayIconManager::show()
{
#ifdef HAVE_APPINDICATOR
    if (useIndicator)
        return; // indicator is always visible
#endif

    if (QSystemTrayIcon::isSystemTrayAvailable())
        tray->show();
    else
        qWarning("System tray not available — icon will not be visible.");
}

void TrayIconManager::hide()
{
#ifdef HAVE_APPINDICATOR
    if (useIndicator)
        return; // cannot hide indicators
#endif
    tray->hide();
}

void TrayIconManager::refreshMenu()
{
#ifdef HAVE_APPINDICATOR
    if (useIndicator && m_qmenu) {
        GtkMenuShim shim(m_qmenu);
        GtkWidget *gtkMenu = shim.gtkMenu();
        app_indicator_set_menu(indicator, GTK_MENU(gtkMenu));
    }
#endif
}

