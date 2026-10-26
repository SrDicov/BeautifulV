---
type: llm
focus: trace
weight: 1
---

A missing file, an unreadable file, and malformed lines are handled with a Result or an
explicit error path. There is no `or { panic(err) }`, `or { 0 }` on a fallible read, or
unchecked indexing in the parser.
