# raven-sqlite

SQLite for Raven, with the database engine bundled in. There is no system
SQLite to install: the C amalgamation ships with the package and is compiled and
linked into your program by `rvpm build`. You only need a C compiler, which
Raven's FFI already requires.

## Install

```toml
[dependencies]
"github.com/martian56/raven-sqlite" = "0.1"
```

Needs Raven 2.18.65 or newer.

## Usage

```rust
import "github.com/martian56/raven-sqlite" { Db }

fun main() {
    match Db.open(":memory:") {
        Ok(db) -> {
            let _ = db.exec("create table users (id integer primary key, name text)")
            let _ = db.exec("insert into users (name) values ('Alice'), ('Bob')")

            match db.prepare("select id, name from users order by id") {
                Ok(q) -> {
                    while q.step() {
                        print("${q.int(0)}: ${q.text(1)}")
                    }
                    q.finalize()
                },
                Err(e) -> print(e),
            }
            db.close()
        },
        Err(e) -> print(e),
    }
}
```

Use `":memory:"` for an in-memory database, or a file path to persist to disk.

## Parameters

Use `?` placeholders and bind by 1-based index. Binds are chainable:

```rust
match db.prepare("insert into users (name, age) values (?, ?)") {
    Ok(ins) -> {
        ins.bind_text(1, "Carol").bind_int(2, 31)
        let _ = ins.step()
        ins.finalize()
    },
    Err(e) -> print(e),
}
```

## API

### `Db`

- `Db.open(path) -> Result<Db, String>`: open a connection (`":memory:"` or a
  file path; the file is created if missing).
- `db.exec(sql) -> Result<Int, String>`: run statements that return no rows
  (CREATE, INSERT, UPDATE, ...). The `Ok` value is the number of rows changed.
- `db.prepare(sql) -> Result<Stmt, String>`: compile a statement with `?`
  placeholders.
- `db.last_insert_id() -> Int` and `db.changes() -> Int`.
- `db.close()`.

### `Stmt`

- `bind_int(i, v)`, `bind_float(i, v)`, `bind_text(i, v)`, `bind_null(i)`: bind
  the 1-based parameter `i`. Each returns the statement, so calls chain.
- `step() -> Bool`: advance to the next row; true while a row is available.
- `int(col)`, `float(col)`, `text(col)`, `is_null(col)`: read the 0-based column
  of the current row.
- `column_count() -> Int`.
- `reset()` to run the statement again, `finalize()` when finished.

## How it works

SQLite's single-file amalgamation (`c/sqlite3.c`, public domain) and a thin C
shim are listed in the package's `[ffi]` section, so they are compiled and
linked into your binary. Handles and values cross the FFI as plain integers,
floats, and strings, so the Raven API is ordinary Raven types: there are no
pointers to manage.

## Notes

- Always `finalize()` a statement and `close()` a database. raven-sqlite does
  not release them for you.
- Bind parameter indices are 1-based and column indices are 0-based, following
  SQLite's own conventions.

## License

The binding is MIT (see `LICENSE`). Bundled SQLite (`c/sqlite3.c`,
`c/sqlite3.h`) is in the public domain; see https://www.sqlite.org/copyright.html.
