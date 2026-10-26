---
type: llm
focus: trace
weight: 1
---

Tests cover invalid input: empty string, missing part, leading zero, non-numeric part,
and an invalid prerelease identifier. Each test asserts the failure rather than
expecting a panic.
