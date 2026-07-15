# Changelog

## 0.2.0 - 2026-07-15

- Add checked statement binding, stepping, reading, reset, and finalization APIs.
- Preserve detailed SQLite errors when opening a database or executing a statement.
- Add busy-timeout configuration and checked connection shutdown.
- Preserve embedded NUL bytes in bound and returned text values.
- Expose parameter counts and result column names.
- Expand lifecycle, transaction, error-boundary, and large-value test coverage.
- Test the binding on both Linux and Windows in CI.

## 0.1.0 - 2026-06-29

- Initial release.
