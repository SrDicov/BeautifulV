---
type: llm
focus: trace
weight: 1
---

REVIEW.md puts the panics and swallowed errors on fallible paths in its highest-priority
category. It names `os.read_file(path) or { panic(err) }` in `load_inventory`, the
`strconv.atoi`/`atof64` defaults in `parse_line`, the unchecked `parts[1]`/`parts[2]`
indexing, and the ignored `os.write_file(...) or {}` in `save`. It explains that these
turn bad input or a missing file into a crash or silent data loss.
