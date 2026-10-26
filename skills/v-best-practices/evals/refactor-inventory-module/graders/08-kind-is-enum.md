---
type: llm
focus: trace
weight: 1
---

The `'expensive'`/`'cheap'` strings are replaced by an enum with exhaustive matching, and
the report uses the enum (with `.str()` or a match) rather than free-form strings.
