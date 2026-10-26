---
type: llm
focus: trace
weight: 1
---

Loading returns `!Config` (or a typed equivalent). Missing file, syntax error, and a
full table each return an error with a message; they are not `-1`/`1` codes or panics.
