---
type: llm
focus: trace
weight: 1
---

`initials` checks for an empty string and handles multi-byte characters, so it neither
panics on `''` nor returns half of a UTF-8 rune. A test covers both cases.
