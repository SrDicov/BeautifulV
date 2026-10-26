---
type: llm
focus: trace
weight: 1
---

REVIEW.md flags `kind()` returning `'expensive'` / `'cheap'` strings as a stringly typed
mode. It suggests an enum with those variants so callers compare typed values and an
exhaustive match, and notes that a typo in a string is not caught by the compiler.
