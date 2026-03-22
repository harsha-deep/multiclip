#pragma once
#include <gtk/gtk.h>

extern gboolean tray_window_hidden;

void tray_init(GtkWindow *window, GApplication *app);
void tray_cleanup(void);