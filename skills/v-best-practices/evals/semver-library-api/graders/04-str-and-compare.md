---
type: llm
focus: trace
weight: 1
---

`Version` implements `.str()` in canonical `MAJOR.MINOR.PATCH[-pre][+build]` form, and a
comparison method (for example `compare(other Version) int` or `lt/gt`) that follows
SemVer precedence with build metadata ignored.
