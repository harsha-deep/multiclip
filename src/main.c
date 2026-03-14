#include <gtk/gtk.h>

static void activate(GtkApplication *app)
{
  GtkWidget *window;
  GtkWidget *label;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "MultiClip");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);

  label = gtk_label_new("Hello MultiClip");

  gtk_window_set_child(GTK_WINDOW(window), label);

  gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
  GtkApplication *app;
  int status;

  app =
      gtk_application_new("com.harsha.multiclip", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

  status = g_application_run(G_APPLICATION(app), argc, argv);

  g_object_unref(app);

  return status;
}
