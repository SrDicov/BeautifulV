---
type: llm
focus: trace
weight: 1
---

The refactor adds tests for the parser, the lookup, the report, and every error path
(missing file, malformed line, bad quantity, bad price, missing key). The tests call the
functions directly, not only through `main`.
