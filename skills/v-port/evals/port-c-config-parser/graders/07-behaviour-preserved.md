---
type: llm
focus: trace
weight: 1
---

The port keeps the documented behaviour: `#` comments, blank lines, surrounding
whitespace trimmed, last duplicate wins, a value may contain `=`, and the 32-entry limit.
Tests assert these, not just the happy path.
