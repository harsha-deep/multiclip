#include "db.h"
#include <sqlite3.h>
#include <time.h>

static sqlite3 *s_db = NULL;

static const char *SCHEMA = "CREATE TABLE IF NOT EXISTS clips ("
                            "  id         INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "  type       INTEGER NOT NULL DEFAULT 0,"
                            "  content    TEXT,"
                            "  blob       BLOB,"
                            "  blob_size  INTEGER NOT NULL DEFAULT 0,"
                            "  created_at INTEGER NOT NULL"
                            ");";

gboolean
db_open(const char *path)
{
  if (sqlite3_open(path, &s_db) != SQLITE_OK)
    goto fail;

  char *err = NULL;
  if (sqlite3_exec(s_db, SCHEMA, NULL, NULL, &err) != SQLITE_OK)
    {
      g_warning("db_open: schema error: %s", err);
      sqlite3_free(err);
      goto fail;
    }

  return TRUE;

fail:
  g_warning("db_open failed: %s", s_db ? sqlite3_errmsg(s_db) : "unknown");
  sqlite3_close(s_db);
  s_db = NULL;
  return FALSE;
}

void
db_close(void)
{
  if (s_db)
    {
      sqlite3_close(s_db);
      s_db = NULL;
    }
}

gint64
db_insert_text(const char *text)
{
  if (!s_db || !text)
    return -1;

  sqlite3_stmt *stmt;
  const char *sql
      = "INSERT INTO clips (type, content, created_at) VALUES (0, ?, ?)";

  if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK)
    return -1;

  sqlite3_bind_text(stmt, 1, text, -1, SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt, 2, (gint64)time(NULL));

  gint64 id = -1;
  if (sqlite3_step(stmt) == SQLITE_DONE)
    id = sqlite3_last_insert_rowid(s_db);
  else
    g_warning("db_insert_text: %s", sqlite3_errmsg(s_db));

  sqlite3_finalize(stmt);
  return id;
}

gint64
db_insert_image(const unsigned char *data, gsize size)
{
  if (!s_db || !data || size == 0)
    return -1;

  sqlite3_stmt *stmt;
  const char *sql = "INSERT INTO clips (type, blob, blob_size, created_at) "
                    "VALUES (1, ?, ?, ?)";

  if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK)
    return -1;

  sqlite3_bind_blob(stmt, 1, data, (int)size, SQLITE_TRANSIENT);
  sqlite3_bind_int64(stmt, 2, (gint64)size);
  sqlite3_bind_int64(stmt, 3, (gint64)time(NULL));

  gint64 id = -1;
  if (sqlite3_step(stmt) == SQLITE_DONE)
    id = sqlite3_last_insert_rowid(s_db);
  else
    g_warning("db_insert_image: %s", sqlite3_errmsg(s_db));

  sqlite3_finalize(stmt);
  return id;
}

GPtrArray *
db_load_recent(guint limit)
{
  GPtrArray *arr
      = g_ptr_array_new_with_free_func((GDestroyNotify)clip_entry_free);

  if (!s_db)
    return arr;

  const char *sql = "SELECT id, type, content, blob, blob_size, created_at "
                    "FROM ("
                    "  SELECT id, type, content, blob, blob_size, created_at "
                    "  FROM clips ORDER BY created_at DESC LIMIT ?"
                    ") ORDER BY created_at ASC";

  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(s_db, sql, -1, &stmt, NULL) != SQLITE_OK)
    return arr;

  sqlite3_bind_int(stmt, 1, (int)limit);

  while (sqlite3_step(stmt) == SQLITE_ROW)
    {
      ClipEntry *e = g_new0(ClipEntry, 1);
      e->id = sqlite3_column_int64(stmt, 0);
      e->type = (ClipType)sqlite3_column_int(stmt, 1);
      e->created_at = sqlite3_column_int64(stmt, 5);

      if (e->type == CLIP_TYPE_TEXT)
        {
          const char *t = (const char *)sqlite3_column_text(stmt, 2);
          e->text = g_strdup(t ? t : "");
        }
      else
        {
          gsize sz = (gsize)sqlite3_column_int64(stmt, 4);
          const void *raw = sqlite3_column_blob(stmt, 3);
          if (raw && sz > 0)
            {
              e->blob = g_memdup2(raw, sz);
              e->blob_size = sz;
            }
        }

      g_ptr_array_add(arr, e);
    }

  sqlite3_finalize(stmt);
  return arr;
}

gboolean
db_delete(gint64 id)
{
  if (!s_db)
    return FALSE;

  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(
          s_db, "DELETE FROM clips WHERE id = ?", -1, &stmt, NULL)
      != SQLITE_OK)
    return FALSE;

  sqlite3_bind_int64(stmt, 1, id);
  gboolean ok = (sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);
  return ok;
}

void
db_clear(void)
{
  if (!s_db)
    return;

  char *err = NULL;
  if (sqlite3_exec(s_db, "DELETE FROM clips", NULL, NULL, &err) != SQLITE_OK)
    {
      g_warning("db_clear: %s", err);
      sqlite3_free(err);
    }
}

void
clip_entry_free(ClipEntry *e)
{
  if (!e)
    return;
  g_free(e->text);
  g_free(e->blob);
  g_free(e);
}

void
db_entries_free(GPtrArray *arr)
{
  if (arr)
    g_ptr_array_unref(arr);
}