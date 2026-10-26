---
type: llm
focus: trace
weight: 1
---

`get` returns `?string` (or `(string, bool)`); a missing key is `none`, not `''` or a
NULL-like sentinel. A present key with an empty value is still present.
