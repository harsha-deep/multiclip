#pragma once

#include <gtk/gtk.h>

static const char *CARD_CSS = "listbox {"
                              "  background: transparent;"
                              "  padding: 6px;"
                              "}"
                              "listbox > row {"
                              "  background: transparent;"
                              "  padding: 0;"
                              "  margin: 0 0 6px 0;"
                              "  border-radius: 10px;"
                              "}"
                              "listbox > row:selected {"
                              "  background: transparent;"
                              "}"
                              "listbox > row:selected .clip-card {"
                              "  border-color: @accent_color;"
                              "  border-width: 2px;"
                              "}"
                              ".clip-card {"
                              "  background-color: @card_bg_color;"
                              "  border-radius: 10px;"
                              "  border: 1px solid alpha(@borders, 0.5);"
                              "  padding: 10px 14px 8px 14px;"
                              "}"
                              ".clip-card:hover {"
                              "  background-color: @card_bg_color_alt;"
                              "}"
                              ".clip-meta {"
                              "  font-size: 0.78em;"
                              "  opacity: 0.45;"
                              "}"
                              ".copy-btn {"
                              "  padding: 2px 6px;"
                              "  min-height: 0;"
                              "  min-width: 0;"
                              "}";

typedef struct
{
  GtkWidget *list_box;
  GPtrArray *history;
  gboolean suppress_next;
} AppData;

typedef struct
{
  GtkWindow *window;
  AppData *app;
} PasteCtx;