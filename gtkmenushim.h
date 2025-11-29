#ifndef GTKMENUSHIM_H
#define GTKMENUSHIM_H
#include "gtk/gtk.h"
#include <qmenu.h>


class GtkMenuShim
{
public:
    GtkMenuShim(QMenu *menu);
    ~GtkMenuShim();

    GtkWidget *gtkMenu() const { return m_gtkMenu; }

private:
    GtkWidget *m_gtkMenu = nullptr;
};

#endif // GTKMENUSHIM_H
