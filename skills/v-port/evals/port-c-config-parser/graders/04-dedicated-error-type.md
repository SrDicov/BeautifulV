---
type: llm
focus: trace
weight: 1
---

The error type implements `msg()` and, where useful, `code()`, or is a sum type of
error variants. The caller can tell the failure causes apart without parsing a string.
