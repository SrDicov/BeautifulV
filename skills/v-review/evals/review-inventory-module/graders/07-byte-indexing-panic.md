---
type: llm
focus: trace
weight: 1
---

REVIEW.md flags `initials` for indexing `name[0]` without checking that the string is
non-empty, and notes that string indexing is by byte, not by rune/character. It suggests
an empty check and a rune-aware or byte-safe implementation.
