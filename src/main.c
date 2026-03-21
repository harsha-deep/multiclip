#include "config.h"
#include <gtk/gtk.h>

typedef struct
{
  GtkWidget *list_box;
  GPtrArray *history;
} AppData;

static void
on_check_updates(GtkButton *btn G_GNUC_UNUSED, gpointer data)
{
  GtkUriLauncher *launcher = gtk_uri_launcher_new(
      "https://github.com/harsha-deep/multiclip/releases");
  gtk_uri_launcher_launch(launcher, GTK_WINDOW(data), NULL, NULL, NULL);
  g_object_unref(launcher);
}

static void
on_about(GSimpleAction *action G_GNUC_UNUSED,
         GVariant *param G_GNUC_UNUSED,
         gpointer data)
{
  GtkWindow *parent = GTK_WINDOW(data);

  GtkWidget *about = gtk_window_new();
  gtk_window_set_title(GTK_WINDOW(about), "About MultiClip");
  gtk_window_set_modal(GTK_WINDOW(about), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(about), parent);
  gtk_window_set_resizable(GTK_WINDOW(about), FALSE);
  // gtk_window_set_default_size(GTK_WINDOW(about), 300, 260);

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_set_margin_top(vbox, 24);
  gtk_widget_set_margin_bottom(vbox, 24);
  gtk_widget_set_margin_start(vbox, 24);
  gtk_widget_set_margin_end(vbox, 24);
  gtk_window_set_child(GTK_WINDOW(about), vbox);

  // GtkWidget *icon = gtk_image_new_from_icon_name("edit-paste-symbolic");
  // gtk_image_set_pixel_size(GTK_IMAGE(icon), 64);
  // gtk_box_append(GTK_BOX(vbox), icon);

  GdkTexture *texture = gdk_texture_new_from_file(
      g_file_new_for_path("data/icons/icon.png"), NULL);
  GtkWidget *icon = gtk_image_new_from_paintable(GDK_PAINTABLE(texture));
  gtk_box_append(GTK_BOX(vbox), icon);
  g_object_unref(texture);

  GtkWidget *name_label = gtk_label_new(NULL);
  gtk_label_set_markup(GTK_LABEL(name_label), "<b>MultiClip</b>");
  gtk_box_append(GTK_BOX(vbox), name_label);

  GtkWidget *version_label = gtk_label_new("Version " APP_VERSION);
  gtk_box_append(GTK_BOX(vbox), version_label);

  GtkWidget *link = gtk_link_button_new_with_label(
      "https://github.com/harsha-deep/multiclip", "GitHub Repository");
  gtk_box_append(GTK_BOX(vbox), link);

  GtkWidget *update_btn = gtk_button_new_with_label("Check for Updates");
  // Pass `about` as the parent window for the URI launcher
  g_signal_connect(update_btn, "clicked", G_CALLBACK(on_check_updates), about);
  gtk_box_append(GTK_BOX(vbox), update_btn);

  gtk_window_present(GTK_WINDOW(about));
}

static void
on_clear_history(GSimpleAction *action G_GNUC_UNUSED,
                 GVariant *param G_GNUC_UNUSED,
                 gpointer data)
{
  AppData *app = (AppData *)data;

  // Clear the history array (frees strings)
  g_ptr_array_set_size(app->history, 0);

  // Remove all children from the list box
  GtkWidget *child;
  while ((child = gtk_widget_get_first_child(app->list_box)) != NULL)
    gtk_list_box_remove(GTK_LIST_BOX(app->list_box), child);
}

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

  GMenu *menubar_model = g_menu_new();

  GMenu *file_menu = g_menu_new();
  g_menu_append(file_menu, "Clear History", "win.clear-history");
  g_menu_append(file_menu, "Quit", "app.quit");
  g_menu_append_submenu(menubar_model, "File", G_MENU_MODEL(file_menu));
  g_object_unref(file_menu);

  GMenu *help_menu = g_menu_new();
  g_menu_append(help_menu, "About", "win.about");
  g_menu_append_submenu(menubar_model, "Help", G_MENU_MODEL(help_menu));
  g_object_unref(help_menu);

  GSimpleAction *clear_action = g_simple_action_new("clear-history", NULL);
  g_signal_connect(
      clear_action, "activate", G_CALLBACK(on_clear_history), data);
  g_action_map_add_action(G_ACTION_MAP(window), G_ACTION(clear_action));
  g_object_unref(clear_action);

  GSimpleAction *about_action = g_simple_action_new("about", NULL);
  g_signal_connect(about_action, "activate", G_CALLBACK(on_about), window);
  g_action_map_add_action(G_ACTION_MAP(window), G_ACTION(about_action));
  g_object_unref(about_action);

  GtkWidget *menubar_widget
      = gtk_popover_menu_bar_new_from_model(G_MENU_MODEL(menubar_model));
  g_object_unref(menubar_model);

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_box_append(GTK_BOX(vbox), menubar_widget);
  gtk_box_append(GTK_BOX(vbox), scroll);
  gtk_widget_set_vexpand(scroll, TRUE);
  gtk_window_set_child(GTK_WINDOW(window), vbox);

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