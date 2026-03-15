#include <gtk/gtk.h>

GtkWidget *label;

static void clipboard_text_received(GObject *source,
                                    GAsyncResult *res,
                                    gpointer data)
{
    GError *error = NULL;
    char *text = gdk_clipboard_read_text_finish(GDK_CLIPBOARD(source), res, &error);

    if (text)
    {
        gtk_label_set_text(GTK_LABEL(label), text);
        g_free(text);
    }
    else if (error)
    {
        g_warning("Clipboard read failed: %s", error->message);
        g_error_free(error);
    }
}

// Called every 500ms
static gboolean poll_clipboard(gpointer data)
{
    GdkClipboard *clipboard = GDK_CLIPBOARD(data);
    gdk_clipboard_read_text_async(clipboard, NULL,
                                  clipboard_text_received,
                                  NULL);
    return G_SOURCE_CONTINUE; // keep repeating
}

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "MultiClip");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);

    label = gtk_label_new("Copy something...");
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_window_set_child(GTK_WINDOW(window), label);

    GdkDisplay *display = gdk_display_get_default();
    GdkClipboard *clipboard = gdk_display_get_clipboard(display);

    // Poll every 250ms
    g_timeout_add(250, poll_clipboard, clipboard);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.harsha.multiclip",
                              G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    return status;
}