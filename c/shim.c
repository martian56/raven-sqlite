/* Thin C shim over SQLite for raven-sqlite.
 *
 * Every handle (sqlite3*, sqlite3_stmt*) crosses the FFI as an int64 holding
 * its address, and every value crosses as int64 / double / const char*, so the
 * Raven side speaks only Int, Float, and CStr. The out-pointer and destructor
 * awkwardness of the real API is handled here. */
#include "sqlite3.h"
#include <stdint.h>

#define DB(h)   ((sqlite3 *)(intptr_t)(h))
#define STMT(h) ((sqlite3_stmt *)(intptr_t)(h))
#define H(p)    ((int64_t)(intptr_t)(p))

/* Open `path` (or ":memory:"). Returns the db handle, or 0 on failure. */
int64_t rsq_open(const char *path) {
    sqlite3 *db = 0;
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 0;
    }
    return H(db);
}

int64_t rsq_close(int64_t db) { return sqlite3_close(DB(db)); }

/* Run SQL that returns no rows. Returns the SQLite result code (0 is OK). */
int64_t rsq_exec(int64_t db, const char *sql) {
    return sqlite3_exec(DB(db), sql, 0, 0, 0);
}

const char *rsq_errmsg(int64_t db) { return sqlite3_errmsg(DB(db)); }
int64_t rsq_changes(int64_t db) { return sqlite3_changes(DB(db)); }
int64_t rsq_last_insert_rowid(int64_t db) { return sqlite3_last_insert_rowid(DB(db)); }

/* Compile `sql` into a statement. Returns the statement handle, or 0. */
int64_t rsq_prepare(int64_t db, const char *sql) {
    sqlite3_stmt *stmt = 0;
    if (sqlite3_prepare_v2(DB(db), sql, -1, &stmt, 0) != SQLITE_OK) return 0;
    return H(stmt);
}

int64_t rsq_step(int64_t stmt) { return sqlite3_step(STMT(stmt)); }
int64_t rsq_reset(int64_t stmt) { return sqlite3_reset(STMT(stmt)); }
int64_t rsq_finalize(int64_t stmt) { return sqlite3_finalize(STMT(stmt)); }
int64_t rsq_column_count(int64_t stmt) { return sqlite3_column_count(STMT(stmt)); }
int64_t rsq_column_type(int64_t stmt, int64_t col) {
    return sqlite3_column_type(STMT(stmt), (int)col);
}
int64_t rsq_column_int(int64_t stmt, int64_t col) {
    return sqlite3_column_int64(STMT(stmt), (int)col);
}
double rsq_column_double(int64_t stmt, int64_t col) {
    return sqlite3_column_double(STMT(stmt), (int)col);
}
/* Never returns NULL: a NULL column reads as the empty string. */
const char *rsq_column_text(int64_t stmt, int64_t col) {
    const unsigned char *t = sqlite3_column_text(STMT(stmt), (int)col);
    return t ? (const char *)t : "";
}

/* Bindings use 1-based indices, the SQLite convention. */
int64_t rsq_bind_int(int64_t stmt, int64_t idx, int64_t v) {
    return sqlite3_bind_int64(STMT(stmt), (int)idx, v);
}
int64_t rsq_bind_double(int64_t stmt, int64_t idx, double v) {
    return sqlite3_bind_double(STMT(stmt), (int)idx, v);
}
int64_t rsq_bind_text(int64_t stmt, int64_t idx, const char *v) {
    /* SQLITE_TRANSIENT makes SQLite copy the text, so the caller may free it
     * immediately after this returns. */
    return sqlite3_bind_text(STMT(stmt), (int)idx, v, -1, SQLITE_TRANSIENT);
}
int64_t rsq_bind_null(int64_t stmt, int64_t idx) {
    return sqlite3_bind_null(STMT(stmt), (int)idx);
}
