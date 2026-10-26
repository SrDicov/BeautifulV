---
type: llm
focus: trace
weight: 1
---

Malformed lines (missing timestamp, level, or message) are skipped and included in the
reported `skipped` count. They do not crash the run or get counted under a level.
