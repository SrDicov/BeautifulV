---
type: llm
focus: trace
weight: 1
---

`save` handles `os.write_file` failure and returns it to the caller (or records it),
instead of `or {}`. `find` returns `?Item` or `(Item, bool)` instead of a zero value that
cannot be distinguished from a real item.
