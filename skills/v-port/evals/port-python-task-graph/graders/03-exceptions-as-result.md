---
type: llm
focus: trace
weight: 1
---

`TaskError` is mapped to a V error (or a typed error enum) returned by `add`/`order`.
A cycle, unknown dependency, and duplicate return an error; none of them calls `panic`.
