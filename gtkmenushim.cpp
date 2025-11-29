#include "gtkmenushim.h"


GtkMenuShim::GtkMenuShim(QMenu *menu)
{
    m_gtkMenu = gtk_menu_new();

    for ( QAction *action : menu->actions() )
    {
        if ( action->isSeparator() )
        {
            GtkWidget *separator = gtk_separator_menu_item_new();
            gtk_menu_shell_append(GTK_MENU_SHELL(m_gtkMenu), separator);
            continue;
        }

        GtkWidget *item = gtk_menu_item_new_with_label(action->text().toUtf8().constData());
        gtk_menu_shell_append(GTK_MENU_SHELL(m_gtkMenu), item);

        g_signal_connect(item, "activate", G_CALLBACK(+[](GtkWidget *, gpointer user_data) {
            QAction *act = static_cast<QAction *>(user_data);
            act->trigger();
        }), action );
    }

    gtk_widget_show_all(m_gtkMenu);
}

GtkMenuShim::~GtkMenuShim()
{
    // Nothing yet
}
