---
type: llm
focus: trace
weight: 1
---

`PORTING.md` maps the Python concepts to V (`TaskError` -> error, `Kind` -> enum,
`Optional[int]` -> `?int`, ordered dict -> V map/array) and records any deliberate
difference.
