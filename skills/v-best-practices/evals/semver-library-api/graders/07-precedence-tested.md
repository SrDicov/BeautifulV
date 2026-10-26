---
type: llm
focus: trace
weight: 1
---

Tests cover SemVer precedence: numeric identifiers compare numerically, prerelease
versions sort before the release, prerelease identifiers compare by the SemVer rules,
and build metadata does not affect comparison.
