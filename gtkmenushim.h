#ifndef GTKMENUSHIM_H
#define GTKMENUSHIM_H

#ifdef signals
#undef signals
#endif
#ifdef slots
#undef slots
#endif

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
