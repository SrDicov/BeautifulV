---
tags: [v-best-practices]
max_turns: 20
timeout_seconds: 900
---

Write a small V library for Semantic Versioning 2.0.0. It should:

- Parse `MAJOR.MINOR.PATCH` with optional `-prerelease` and `+build` parts.
- Expose a `Version` type with private fields and accessor methods.
- Return a dedicated error type for invalid input: empty parts, leading zeros,
  non-numeric parts, and invalid identifiers.
- Implement `.str()` so a parsed version prints back in canonical form.
- Provide a comparison method that follows SemVer precedence (build metadata ignored).
- Document every public item, including the error contract.
- Include tests for valid versions, precedence, and each failure mode.
- Compile as a library module (not `main`) with a `v.mod`.

Use V 0.5.x. Do not use `panic`, `assert`, or a third-party dependency for parsing.
