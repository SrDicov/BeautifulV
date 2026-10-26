---
type: llm
focus: trace
weight: 1
---

A cycle and an unknown dependency are reported as errors from `order()` (or from the
operation that validates the graph), with enough context to identify the task. The
process does not abort.
