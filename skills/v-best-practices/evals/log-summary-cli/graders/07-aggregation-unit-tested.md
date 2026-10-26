---
type: llm
focus: trace
weight: 1
---

The aggregation and formatting live in functions that are unit-tested directly, not only
through a subprocess. Tests cover a normal file, malformed lines, and an empty file.
