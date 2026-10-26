---
type: llm
focus: trace
weight: 1
---

The port handles the edge cases the C code defines: a key with no `=`, an empty key, a
too-long key/value, and the 32-entry limit. Each returns the corresponding error instead
of truncating silently or panicking.
