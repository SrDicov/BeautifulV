---
type: llm
focus: trace
weight: 1
---

`Version` has private fields (no `pub:` or `pub mut:` on the data), and callers read
them through accessor methods such as `major()`, `minor()`, `patch()`, `prerelease()`,
or `build()`.
