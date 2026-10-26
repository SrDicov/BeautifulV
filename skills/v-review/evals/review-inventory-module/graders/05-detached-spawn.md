---
type: llm
focus: trace
weight: 1
---

REVIEW.md flags `save_async` for discarding the handle returned by `spawn`. It explains
that the thread is detached, that the program can exit before the write finishes, and
that the handle must be stored and waited on, or the work must use a managed worker
pattern.
