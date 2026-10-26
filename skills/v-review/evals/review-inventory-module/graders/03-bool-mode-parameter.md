---
type: llm
focus: trace
weight: 1
---

REVIEW.md flags the `force bool` parameter of `add_item`. It explains that `true`/`false`
at the call site does not say what the flag means, and suggests an enum (for example
`.force` / `.reject`) or two functions.
