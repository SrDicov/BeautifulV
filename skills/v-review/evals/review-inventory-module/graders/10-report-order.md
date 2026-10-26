---
type: llm
focus: trace
weight: 1
---

REVIEW.md notes that `report` emits items in map/file insertion order, so the same set of
items in a different file order produces a different report. It asks for an explicit sort
(by name, or another documented key) if the report is used for comparison, snapshots, or
user-facing output.
