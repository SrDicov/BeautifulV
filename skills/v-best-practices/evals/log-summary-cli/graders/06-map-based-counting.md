---
type: llm
focus: trace
weight: 1
---

Counting uses a `map[string]int` (or an equivalent single-pass aggregation) rather than a
hard-coded counter for each level or repeated scans of the file.
