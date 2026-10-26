---
type: llm
focus: trace
weight: 1
---

`save_async` no longer discards the `spawn` handle. The thread is waited on, the API
returns the handle, or the asynchronous save is removed in favour of a synchronous call.
The agent explains the lifetime/exit risk.
