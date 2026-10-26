---
type: llm
focus: trace
weight: 1
---

The parser and comparison code contain no `panic`, `assert`, `or { panic(...) }`, or
unchecked numeric conversion of untrusted parts. Invalid input returns the error type.
