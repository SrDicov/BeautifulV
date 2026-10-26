---
type: llm
focus: trace
weight: 1
---

The V module compiles and `v test .` passes. The module is a library (`-shared` for
`v -check`), not a `main` program that merely runs the parser once.
