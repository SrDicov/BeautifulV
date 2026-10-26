---
type: llm
focus: trace
weight: 1
---

Parsing returns `!Version` (or an equivalent Result) with a dedicated error type that
implements `msg()`; the error is not a bare string, `panic`, or a sentinel value. The
error type documents or exposes enough information to tell why parsing failed.
