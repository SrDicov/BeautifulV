---
type: llm
focus: trace
weight: 1
---

REVIEW.md flags that the module has no tests. It notes that `parse_line`, the CSV
round-trip, `find`, and every error path are untested, and asks for `_test.v` cases that
call those functions directly (including malformed lines, a missing file, and a bad
quantity/price).
