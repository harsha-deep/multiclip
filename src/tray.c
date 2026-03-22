#include "tray.h"
#include <gio/gio.h>
#include <unistd.h>

static const char SNI_XML[]
    = "<node>"
      "  <interface name='org.kde.StatusNotifierItem'>"
      "    <property name='Category' type='s' access='read'/>"
      "    <property name='Id'       type='s' access='read'/>"
      "    <property name='Title'    type='s' access='read'/>"
      "    <property name='Status'   type='s' access='read'/>"
      "    <property name='IconName' type='s' access='read'/>"
      "    <property name='Menu'     type='o' access='read'/>"
      "    <method name='Activate'>"
      "      <arg name='x' type='i' direction='in'/>"
      "      <arg name='y' type='i' direction='in'/>"
      "    </method>"
      "    <method name='SecondaryActivate'>"
      "      <arg name='x' type='i' direction='in'/>"
      "      <arg name='y' type='i' direction='in'/>"
      "    </method>"
      "    <method name='ContextMenu'>"
      "      <arg name='x' type='i' direction='in'/>"
      "      <arg name='y' type='i' direction='in'/>"
      "    </method>"
      "  </interface>"
      "</node>";

static GDBusNodeInfo *s_introspection = NULL;
static guint s_registration = 0;
static guint s_bus_name_id = 0;
static GtkWindow *s_window = NULL;
static GApplication *s_app = NULL;

gboolean tray_window_hidden = FALSE;

static void
handle_method_call(GDBusConnection *conn G_GNUC_UNUSED,
                   const char *sender G_GNUC_UNUSED,
                   const char *obj_path G_GNUC_UNUSED,
                   const char *iface G_GNUC_UNUSED,
                   const char *method,
                   GVariant *params G_GNUC_UNUSED,
                   GDBusMethodInvocation *invocation G_GNUC_UNUSED,
                   gpointer user_data G_GNUC_UNUSED)
{
  if (g_strcmp0(method, "Activate") == 0
      || g_strcmp0(method, "SecondaryActivate") == 0
      || g_strcmp0(method, "ContextMenu") == 0)
    {
      if (tray_window_hidden)
        {
          tray_window_hidden = FALSE;
          gtk_widget_set_visible(GTK_WIDGET(s_window), TRUE);
          gtk_window_unminimize(s_window);
          gtk_window_present(s_window);
        }
      else
        {
          tray_window_hidden = TRUE;
          gtk_widget_set_visible(GTK_WIDGET(s_window), FALSE);
        }
    }
}

static GVariant *
handle_get_property(GDBusConnection *conn G_GNUC_UNUSED,
                    const char *sender G_GNUC_UNUSED,
                    const char *obj_path G_GNUC_UNUSED,
                    const char *iface G_GNUC_UNUSED,
                    const char *property,
                    GError **error G_GNUC_UNUSED,
                    gpointer user_data G_GNUC_UNUSED)
{
  if (g_strcmp0(property, "Category") == 0)
    return g_variant_new_string("ApplicationStatus");
  if (g_strcmp0(property, "Id") == 0)
    return g_variant_new_string("multiclip");
  if (g_strcmp0(property, "Title") == 0)
    return g_variant_new_string("MultiClip");
  if (g_strcmp0(property, "Status") == 0)
    return g_variant_new_string("Active");
  if (g_strcmp0(property, "IconName") == 0)
    return g_variant_new_string("edit-paste-symbolic");
  if (g_strcmp0(property, "Menu") == 0)
    return g_variant_new_object_path("/NO_DBUSMENU");
  return NULL;
}

static const GDBusInterfaceVTable s_vtable = {
  .method_call = handle_method_call,
  .get_property = handle_get_property,
  .set_property = NULL,
};

static void
on_bus_acquired(GDBusConnection *conn,
                const char *name,
                gpointer user_data G_GNUC_UNUSED)
{
  GError *error = NULL;

  s_registration
      = g_dbus_connection_register_object(conn,
                                          "/StatusNotifierItem",
                                          s_introspection->interfaces[0],
                                          &s_vtable,
                                          NULL,
                                          NULL,
                                          &error);
  if (error)
    {
      g_warning("tray: register object failed: %s", error->message);
      g_clear_error(&error);
      return;
    }

  g_dbus_connection_call(conn,
                         "org.kde.StatusNotifierWatcher",
                         "/StatusNotifierWatcher",
                         "org.kde.StatusNotifierWatcher",
                         "RegisterStatusNotifierItem",
                         g_variant_new("(s)", name),
                         NULL,
                         G_DBUS_CALL_FLAGS_NONE,
                         -1,
                         NULL,
                         NULL,
                         NULL);
}

static void
on_name_acquired(GDBusConnection *c G_GNUC_UNUSED,
                 const char *n G_GNUC_UNUSED,
                 gpointer d G_GNUC_UNUSED)
{
}

static void
on_name_lost(GDBusConnection *c G_GNUC_UNUSED,
             const char *n G_GNUC_UNUSED,
             gpointer d G_GNUC_UNUSED)
{
  g_warning("tray: could not own bus name — tray icon unavailable");
}

void
tray_init(GtkWindow *window, GApplication *app)
{
  s_window = window;
  s_app = app;

  GError *error = NULL;
  s_introspection = g_dbus_node_info_new_for_xml(SNI_XML, &error);
  if (error)
    {
      g_warning("tray: XML parse error: %s", error->message);
      g_clear_error(&error);
      return;
    }

  char *bus_name
      = g_strdup_printf("org.kde.StatusNotifierItem-%d-1", (int)getpid());

  s_bus_name_id = g_bus_own_name(G_BUS_TYPE_SESSION,
                                 bus_name,
                                 G_BUS_NAME_OWNER_FLAGS_NONE,
                                 on_bus_acquired,
                                 on_name_acquired,
                                 on_name_lost,
                                 NULL,
                                 NULL);
  g_free(bus_name);
}

void
tray_cleanup(void)
{
  if (s_bus_name_id)
    {
      g_bus_unown_name(s_bus_name_id);
      s_bus_name_id = 0;
    }
  if (s_introspection)
    {
      g_dbus_node_info_unref(s_introspection);
      s_introspection = NULL;
    }
}