---
type: llm
focus: trace
weight: 1
---

`PORTING.md` maps the C API to the V API (`config_load` -> `Config.load`, `config_get` ->
`Config.get`, error codes -> `!Config`) and lists any deliberate difference such as error
messages or capacity handling.
