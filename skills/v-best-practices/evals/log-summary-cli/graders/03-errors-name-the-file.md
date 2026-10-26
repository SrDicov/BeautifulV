---
type: llm
focus: trace
weight: 1
---

A read failure prints an error to stderr that contains the path the user supplied, and
the process exits non-zero. It does not print a bare `error` or a stack trace.
