#include <gtk/gtk.h>

// Your app state - NO MORE GLOBALS (except this one struct)
typedef struct
{
  GtkWidget *list_box;
  GPtrArray *history;
} AppData;

static void
add_to_history(AppData *app, const char *text)
{
  // Dedup: don't add if same as last entry
  if (app->history->len > 0)
    {
      const char *last = g_ptr_array_index(app->history, app->history->len - 1);
      if (g_strcmp0(last, text) == 0)
        return;
    }

  // Cap at 50 entries
  if (app->history->len >= 50)
    g_ptr_array_remove_index(app->history, 0);

  // Store a copy of the string
  g_ptr_array_add(app->history, g_strdup(text));

  // Add a row to the visual list
  GtkWidget *label = gtk_label_new(text);
  gtk_label_set_xalign(GTK_LABEL(label), 0.0f); // left align
  gtk_label_set_max_width_chars(GTK_LABEL(label), 60);
  gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);

  gtk_list_box_prepend(GTK_LIST_BOX(app->list_box), label);
  gtk_widget_set_visible(label, TRUE);
}

static void
clipboard_text_received(GObject *source, GAsyncResult *res, gpointer data)
{
  AppData *app = (AppData *)data; // cast your struct back
  GError *error = NULL;

  char *text
      = gdk_clipboard_read_text_finish(GDK_CLIPBOARD(source), res, &error);

  if (text)
    {
      add_to_history(app, text);
      g_free(text);
    }
  else if (error)
    {
      g_warning("Clipboard read failed: %s", error->message);
      g_error_free(error);
    }
}

static void
clipboard_changed(GdkClipboard *clipboard, gpointer data)
{
  // Pass your AppData through to the callback
  gdk_clipboard_read_text_async(clipboard, NULL, clipboard_text_received, data);
}

static void
activate(GtkApplication *app, gpointer user_data G_GNUC_UNUSED)
{
  // Allocate app state
  AppData *data = g_new0(AppData, 1);
  data->history = g_ptr_array_new_with_free_func(g_free); // auto-frees strings

  // Window
  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "MultiClip");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);

  // Scrollable container
  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(
      GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  // The list
  data->list_box = gtk_list_box_new();
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(data->list_box),
                                  GTK_SELECTION_SINGLE);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), data->list_box);
  gtk_window_set_child(GTK_WINDOW(window), scroll);

  // Clipboard
  GdkDisplay *display = gdk_display_get_default();
  GdkClipboard *clipboard = gdk_display_get_clipboard(display);
  g_signal_connect(clipboard, "changed", G_CALLBACK(clipboard_changed), data);

  gtk_window_present(GTK_WINDOW(window));
}

int
main(int argc, char **argv)
{
  GtkApplication *app = gtk_application_new("com.harsha.multiclip",
                                            G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);
  return status;
}