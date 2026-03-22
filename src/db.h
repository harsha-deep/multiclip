#pragma once
#include <glib.h>

typedef enum
{
  CLIP_TYPE_TEXT = 0,
  CLIP_TYPE_IMAGE = 1,
} ClipType;

typedef struct
{
  gint64 id;
  ClipType type;
  char *text;
  unsigned char *blob;
  gsize blob_size;
  gint64 created_at;
} ClipEntry;

gboolean db_open(const char *path);
void db_close(void);

gint64 db_insert_text(const char *text);
gint64 db_insert_image(const unsigned char *data, gsize size);

GPtrArray *db_load_recent(guint limit);

gboolean db_delete(gint64 id);
void db_clear(void);

void clip_entry_free(ClipEntry *entry);
void db_entries_free(GPtrArray *arr);