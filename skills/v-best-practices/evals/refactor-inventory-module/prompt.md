---
tags: [v-best-practices]
max_turns: 30
timeout_seconds: 1200
---

Refactor `inventory.v` so it is idiomatic, safe V without changing the information a
successful run prints. A contractor wrote it; keep the good parts.

Requirements:

- No panic or silent default on a fallible path. Return correct Option/Result values.
- Do not expose state that must stay consistent. Use private fields and a validating
  constructor or factory.
- Remove the bool mode parameter in favour of a typed choice.
- Wait for, or remove, the detached background save.
- Make `initials` safe for empty and non-ASCII names.
- Replace the stringly typed item kind with an enum.
- Make the report order deterministic.
- Document the public API so `v vet -W` passes.
- Add tests for the parser, the lookup, the report, and the error paths.
- Write `CHANGES.md` listing each change with its reason, and call out any signature or
  behaviour change.

Input files: inventory.v

Use V 0.5.x. Finish with `v fmt -verify`, `v -check`, `v vet -W`, and `v test`.
