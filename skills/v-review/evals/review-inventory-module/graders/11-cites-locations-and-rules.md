---
type: llm
focus: trace
weight: 1
---

Every finding in REVIEW.md gives a `file:line` location and names the rule or language
guarantee it breaks (for example `?T` versus `!T`, mutability, bool parameters, spawn
handles, or `v vet` documentation). The review groups repeated instances of the same
mistake instead of listing the same problem once per line.
