#include "main.h"
#include "config.h"
#include "db.h"
#include "tray.h"

#ifdef GDK_WINDOWING_X11
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <gdk/x11/gdkx.h>
#endif

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

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_widget_set_margin_top(vbox, 24);
  gtk_widget_set_margin_bottom(vbox, 24);
  gtk_widget_set_margin_start(vbox, 24);
  gtk_widget_set_margin_end(vbox, 24);
  gtk_window_set_child(GTK_WINDOW(about), vbox);

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

  db_clear();

  g_ptr_array_set_size(app->history, 0);

  GtkWidget *child;
  while ((child = gtk_widget_get_first_child(app->list_box)) != NULL)
    gtk_list_box_remove(GTK_LIST_BOX(app->list_box), child);
}

static void
on_dark_mode_change_state(GSimpleAction *action,
                          GVariant *state,
                          gpointer data G_GNUC_UNUSED)
{
  g_simple_action_set_state(action, state);
  gboolean dark = g_variant_get_boolean(state);
  g_object_set(gtk_settings_get_default(),
               "gtk-application-prefer-dark-theme",
               dark,
               NULL);
}

static void
on_copy_btn_clicked(GtkButton *btn, gpointer data G_GNUC_UNUSED)
{
  const char *text = g_object_get_data(G_OBJECT(btn), "clip-text");
  GdkClipboard *cb = gdk_display_get_clipboard(gdk_display_get_default());
  gdk_clipboard_set_text(cb, text);
}

static gboolean
do_paste(gpointer data)
{
  PasteCtx *ctx = data;

  ctx->app->suppress_next = TRUE;

  if (!g_spawn_command_line_async("xdotool key --clearmodifiers ctrl+v", NULL))
    g_spawn_command_line_async("wtype -M ctrl v -m ctrl", NULL);

  g_free(ctx);
  return G_SOURCE_REMOVE;
}

static void
on_row_activated(GtkListBox *box G_GNUC_UNUSED,
                 GtkListBoxRow *row,
                 gpointer data)
{
  AppData *app = (AppData *)data;
  GtkWindow *window
      = GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(app->list_box)));

  const char *text = g_object_get_data(G_OBJECT(row), "clip-text");
  GdkTexture *texture = g_object_get_data(G_OBJECT(row), "clip-texture");

  if (!text && !texture)
    return;

  GdkClipboard *cb = gdk_display_get_clipboard(gdk_display_get_default());

  if (text)
    gdk_clipboard_set_text(cb, text);
  else
    gdk_clipboard_set_texture(cb, texture);

  gtk_window_minimize(window);
  tray_window_hidden = TRUE;

  PasteCtx *ctx = g_new0(PasteCtx, 1);
  ctx->window = window;
  ctx->app = app;
  g_timeout_add(200, do_paste, ctx);
}

static GtkWidget *
make_clip_card(const char *text)
{
  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_add_css_class(card, "clip-card");

  GtkWidget *label = gtk_label_new(text);
  gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
  gtk_label_set_max_width_chars(GTK_LABEL(label), 55);
  gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
  gtk_label_set_lines(GTK_LABEL(label), 3);
  gtk_label_set_wrap(GTK_LABEL(label), TRUE);
  gtk_label_set_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
  gtk_widget_set_hexpand(label, TRUE);
  gtk_box_append(GTK_BOX(card), label);

  GtkWidget *bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_append(GTK_BOX(card), bottom);

  GtkWidget *meta = gtk_label_new(g_strdup_printf("%zu chars", strlen(text)));
  gtk_widget_add_css_class(meta, "clip-meta");
  gtk_label_set_xalign(GTK_LABEL(meta), 0.0f);
  gtk_widget_set_hexpand(meta, TRUE);
  gtk_box_append(GTK_BOX(bottom), meta);

  GtkWidget *copy_btn = gtk_button_new_from_icon_name("edit-copy-symbolic");
  gtk_widget_add_css_class(copy_btn, "flat");
  gtk_widget_add_css_class(copy_btn, "copy-btn");
  gtk_widget_set_tooltip_text(copy_btn, "Copy to clipboard");
  g_object_set_data_full(
      G_OBJECT(copy_btn), "clip-text", g_strdup(text), g_free);
  g_signal_connect(copy_btn, "clicked", G_CALLBACK(on_copy_btn_clicked), NULL);
  gtk_box_append(GTK_BOX(bottom), copy_btn);

  return card;
}

static GtkWidget *
make_image_card(GdkTexture *texture, const char *name)
{
  GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_add_css_class(card, "clip-card");

  GtkWidget *picture = gtk_picture_new_for_paintable(GDK_PAINTABLE(texture));
  gtk_picture_set_content_fit(GTK_PICTURE(picture), GTK_CONTENT_FIT_CONTAIN);
  gtk_picture_set_can_shrink(GTK_PICTURE(picture), TRUE);
  gtk_widget_set_size_request(picture, -1, 200);
  gtk_box_append(GTK_BOX(card), picture);

  GtkWidget *label = gtk_label_new(name);
  gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
  gtk_label_set_max_width_chars(GTK_LABEL(label), 55);
  gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_MIDDLE);
  gtk_widget_add_css_class(label, "clip-meta");
  gtk_box_append(GTK_BOX(card), label);

  return card;
}

static void
add_image_to_history(AppData *app,
                     GdkTexture *texture,
                     const char *name,
                     gint64 row_id)
{
  GtkWidget *card = make_image_card(texture, name);
  GtkListBoxRow *row = GTK_LIST_BOX_ROW(gtk_list_box_row_new());
  gtk_list_box_row_set_child(row, card);

  g_object_set_data_full(
      G_OBJECT(row), "clip-texture", g_object_ref(texture), g_object_unref);

  if (row_id >= 0)
    {
      gint64 *idp = g_new(gint64, 1);
      *idp = row_id;
      g_object_set_data_full(G_OBJECT(row), "clip-id", idp, g_free);
    }

  gtk_list_box_prepend(GTK_LIST_BOX(app->list_box), GTK_WIDGET(row));
  gtk_widget_set_visible(GTK_WIDGET(row), TRUE);
}

static void
add_to_history(AppData *app, const char *text, gboolean persist)
{
  if (app->history->len > 0)
    {
      const char *last = g_ptr_array_index(app->history, app->history->len - 1);
      if (g_strcmp0(last, text) == 0)
        return;
    }

  if (app->history->len >= 50)
    {
      GtkWidget *last_child = gtk_widget_get_last_child(app->list_box);
      if (last_child)
        {
          GtkListBoxRow *old_row = GTK_LIST_BOX_ROW(last_child);
          gint64 *old_id = g_object_get_data(G_OBJECT(old_row), "clip-id");
          if (old_id)
            db_delete(*old_id);
          gtk_list_box_remove(GTK_LIST_BOX(app->list_box), last_child);
        }

      g_ptr_array_remove_index(app->history, 0);
    }

  g_ptr_array_add(app->history, g_strdup(text));

  gint64 row_id = persist ? db_insert_text(text) : -1;

  GtkWidget *card = make_clip_card(text);
  GtkListBoxRow *row = GTK_LIST_BOX_ROW(gtk_list_box_row_new());
  gtk_list_box_row_set_child(row, card);

  g_object_set_data_full(G_OBJECT(row), "clip-text", g_strdup(text), g_free);

  if (row_id >= 0)
    {
      gint64 *idp = g_new(gint64, 1);
      *idp = row_id;
      g_object_set_data_full(G_OBJECT(row), "clip-id", idp, g_free);
    }

  gtk_list_box_prepend(GTK_LIST_BOX(app->list_box), GTK_WIDGET(row));
  gtk_widget_set_visible(GTK_WIDGET(row), TRUE);
}

static gboolean
on_drop(GtkDropTarget *target G_GNUC_UNUSED,
        const GValue *value,
        double x G_GNUC_UNUSED,
        double y G_GNUC_UNUSED,
        gpointer data)
{
  if (!G_VALUE_HOLDS(value, GDK_TYPE_FILE_LIST))
    return FALSE;

  AppData *app = (AppData *)data;
  GSList *files = gdk_file_list_get_files(g_value_get_boxed(value));
  gboolean handled = FALSE;

  for (GSList *l = files; l != NULL; l = l->next)
    {
      GFile *file = G_FILE(l->data);
      char *path = g_file_get_path(file);
      if (!path)
        continue;

      gchar *lower = g_ascii_strdown(path, -1);
      gboolean is_image = g_str_has_suffix(lower, ".jpg")
                          || g_str_has_suffix(lower, ".jpeg")
                          || g_str_has_suffix(lower, ".png")
                          || g_str_has_suffix(lower, ".gif");
      g_free(lower);

      if (is_image)
        {
          GError *error = NULL;
          GBytes *bytes = g_file_load_bytes(file, NULL, NULL, &error);
          if (!bytes)
            {
              g_warning("on_drop: cannot read '%s': %s",
                        path,
                        error ? error->message : "unknown");
              g_clear_error(&error);
              g_free(path);
              continue;
            }

          GdkTexture *texture = gdk_texture_new_from_bytes(bytes, &error);
          if (!texture)
            {
              g_warning("on_drop: cannot decode '%s': %s",
                        path,
                        error ? error->message : "unknown");
              g_clear_error(&error);
              g_bytes_unref(bytes);
              g_free(path);
              continue;
            }

          char *name = g_path_get_basename(path);
          gsize data_size;
          gconstpointer raw = g_bytes_get_data(bytes, &data_size);
          gint64 row_id
              = db_insert_image((const unsigned char *)raw, data_size);

          add_image_to_history(app, texture, name, row_id);

          g_free(name);
          g_bytes_unref(bytes);
          g_object_unref(texture);
          handled = TRUE;
        }

      g_free(path);
    }

  return handled;
}

static void
clipboard_text_received(GObject *source, GAsyncResult *res, gpointer data)
{
  AppData *app = (AppData *)data;
  GError *error = NULL;

  char *text
      = gdk_clipboard_read_text_finish(GDK_CLIPBOARD(source), res, &error);

  if (text)
    {
      if (app->suppress_next)
        app->suppress_next = FALSE;
      else
        add_to_history(app, text, TRUE);
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
  gdk_clipboard_read_text_async(clipboard, NULL, clipboard_text_received, data);
}

static gboolean
on_window_close_request(GtkWindow *window G_GNUC_UNUSED,
                        gpointer data G_GNUC_UNUSED)
{
  gtk_widget_set_visible(GTK_WIDGET(window), FALSE);
  tray_window_hidden = TRUE;
  return TRUE;
}

#ifdef GDK_WINDOWING_X11

static GtkWindow *s_hotkey_window = NULL;

static gboolean
hotkey_toggle_idle(gpointer data G_GNUC_UNUSED)
{
  if (!s_hotkey_window)
    return G_SOURCE_REMOVE;
  if (gtk_widget_get_visible(GTK_WIDGET(s_hotkey_window)))
    gtk_widget_set_visible(GTK_WIDGET(s_hotkey_window), FALSE);
  else
    gtk_window_present(s_hotkey_window);
  return G_SOURCE_REMOVE;
}

static gpointer
hotkey_thread(gpointer data G_GNUC_UNUSED)
{
  Display *d = XOpenDisplay(NULL);
  if (!d)
    {
      g_warning("hotkey: XOpenDisplay failed — Super+V disabled");
      return NULL;
    }

  Window root = DefaultRootWindow(d);
  KeyCode kc = XKeysymToKeycode(d, XK_v);
  int mods = Mod4Mask;

  XGrabKey(d, kc, mods, root, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(d, kc, mods | Mod2Mask, root, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(d, kc, mods | LockMask, root, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(d,
           kc,
           mods | Mod2Mask | LockMask,
           root,
           True,
           GrabModeAsync,
           GrabModeAsync);
  XSelectInput(d, root, KeyPressMask);
  XFlush(d);

  for (;;)
    {
      XEvent e;
      XNextEvent(d, &e);
      if (e.type == KeyPress && e.xkey.keycode == kc)
        g_idle_add(hotkey_toggle_idle, NULL);
    }

  XCloseDisplay(d);
  return NULL;
}

static void
setup_x11_hotkey(GtkWindow *window)
{
  if (!GDK_IS_X11_DISPLAY(gdk_display_get_default()))
    {
      g_message("hotkey: running under native Wayland — Super+V disabled");
      return;
    }
  s_hotkey_window = window;
  g_thread_new("multiclip-hotkey", hotkey_thread, NULL);
}

#endif

static void
activate(GtkApplication *app, gpointer user_data G_GNUC_UNUSED)
{
  char *data_dir = g_build_filename(g_get_user_data_dir(), "multiclip", NULL);
  g_mkdir_with_parents(data_dir, 0700);
  char *db_path = g_build_filename(data_dir, "history.db", NULL);
  g_free(data_dir);

  if (!db_open(db_path))
    g_warning("Could not open database at %s — history will not be saved",
              db_path);
  g_free(db_path);

  AppData *data = g_new0(AppData, 1);
  data->history = g_ptr_array_new_with_free_func(g_free);
  data->suppress_next = FALSE;

  GtkWidget *window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "MultiClip");
  gtk_window_set_default_size(GTK_WINDOW(window), 420, 540);

  GtkCssProvider *css = gtk_css_provider_new();
  gtk_css_provider_load_from_string(css, CARD_CSS);
  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(),
      GTK_STYLE_PROVIDER(css),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(css);

  GSimpleAction *clear_action = g_simple_action_new("clear-history", NULL);
  g_signal_connect(
      clear_action, "activate", G_CALLBACK(on_clear_history), data);
  g_action_map_add_action(G_ACTION_MAP(window), G_ACTION(clear_action));
  g_object_unref(clear_action);

  GSimpleAction *about_action = g_simple_action_new("about", NULL);
  g_signal_connect(about_action, "activate", G_CALLBACK(on_about), window);
  g_action_map_add_action(G_ACTION_MAP(window), G_ACTION(about_action));
  g_object_unref(about_action);

  GSimpleAction *dark_action = g_simple_action_new_stateful(
      "dark-mode", NULL, g_variant_new_boolean(FALSE));
  g_signal_connect(
      dark_action, "change-state", G_CALLBACK(on_dark_mode_change_state), NULL);
  g_action_map_add_action(G_ACTION_MAP(window), G_ACTION(dark_action));
  g_object_unref(dark_action);

  GSimpleAction *quit_action = g_simple_action_new("quit", NULL);
  g_signal_connect_swapped(
      quit_action, "activate", G_CALLBACK(tray_cleanup), NULL);
  g_signal_connect_swapped(quit_action, "activate", G_CALLBACK(db_close), NULL);
  g_signal_connect_swapped(
      quit_action, "activate", G_CALLBACK(g_application_quit), app);
  g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(quit_action));
  g_object_unref(quit_action);

  GMenu *menu_model = g_menu_new();

  GMenu *actions_section = g_menu_new();
  g_menu_append(actions_section, "Clear History", "win.clear-history");
  g_menu_append_section(menu_model, NULL, G_MENU_MODEL(actions_section));
  g_object_unref(actions_section);

  GMenu *view_section = g_menu_new();
  g_menu_append(view_section, "Prefer Dark Theme", "win.dark-mode");
  g_menu_append_section(menu_model, NULL, G_MENU_MODEL(view_section));
  g_object_unref(view_section);

  GMenu *app_section = g_menu_new();
  g_menu_append(app_section, "About", "win.about");
  g_menu_append(app_section, "Quit", "app.quit");
  g_menu_append_section(menu_model, NULL, G_MENU_MODEL(app_section));
  g_object_unref(app_section);

  GtkWidget *header = gtk_header_bar_new();
  GtkWidget *menu_button = gtk_menu_button_new();
  gtk_menu_button_set_icon_name(GTK_MENU_BUTTON(menu_button),
                                "open-menu-symbolic");
  gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menu_button),
                                 G_MENU_MODEL(menu_model));
  g_object_unref(menu_model);
  gtk_header_bar_pack_end(GTK_HEADER_BAR(header), menu_button);
  gtk_window_set_titlebar(GTK_WINDOW(window), header);

  GtkWidget *scroll = gtk_scrolled_window_new();
  gtk_scrolled_window_set_policy(
      GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  data->list_box = gtk_list_box_new();
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(data->list_box),
                                  GTK_SELECTION_SINGLE);
  gtk_widget_add_css_class(data->list_box, "background");

  g_signal_connect(
      data->list_box, "row-activated", G_CALLBACK(on_row_activated), data);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), data->list_box);
  gtk_widget_set_vexpand(scroll, TRUE);
  gtk_window_set_child(GTK_WINDOW(window), scroll);

  GdkDisplay *display = gdk_display_get_default();
  GdkClipboard *clipboard = gdk_display_get_clipboard(display);
  g_signal_connect(clipboard, "changed", G_CALLBACK(clipboard_changed), data);

  GtkDropTarget *drop_target
      = gtk_drop_target_new(GDK_TYPE_FILE_LIST, GDK_ACTION_COPY);
  g_signal_connect(drop_target, "drop", G_CALLBACK(on_drop), data);
  gtk_widget_add_controller(GTK_WIDGET(window),
                            GTK_EVENT_CONTROLLER(drop_target));

  gtk_window_present(GTK_WINDOW(window));

  g_signal_connect(
      window, "close-request", G_CALLBACK(on_window_close_request), NULL);

  tray_init(GTK_WINDOW(window), G_APPLICATION(app));

#ifdef GDK_WINDOWING_X11
  setup_x11_hotkey(GTK_WINDOW(window));
#endif

  GPtrArray *saved = db_load_recent(50);
  for (guint i = 0; i < saved->len; i++)
    {
      ClipEntry *e = g_ptr_array_index(saved, i);
      if (e->type == CLIP_TYPE_TEXT && e->text)
        add_to_history(data, e->text, FALSE);
      else if (e->type == CLIP_TYPE_IMAGE && e->blob && e->blob_size > 0)
        {
          GBytes *bytes = g_bytes_new(e->blob, e->blob_size);
          GdkTexture *texture = gdk_texture_new_from_bytes(bytes, NULL);
          g_bytes_unref(bytes);
          if (texture)
            {
              add_image_to_history(data, texture, "Saved image", e->id);
              g_object_unref(texture);
            }
        }
    }
  db_entries_free(saved);
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