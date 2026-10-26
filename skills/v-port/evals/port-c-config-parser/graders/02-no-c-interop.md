---
type: llm
focus: trace
weight: 1
---

The port is pure V. It does not `#include` `config.h`, declare `C.config_load`, link
`config.c`, or otherwise call the original implementation. No `unsafe` is needed for
the parser.
