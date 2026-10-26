---
tags: [v-port]
max_turns: 30
timeout_seconds: 1200
---

Port `config.c` and `config.h` to V. Keep the behaviour of the C implementation and port
`test_config.c` first, so the V tests prove the same contract.

Requirements:

- A pure V module; do not call the C implementation.
- `Config.load(path) !Config` instead of an error code, with a dedicated error type.
- `Config.get(key) ?string` instead of NULL.
- Keep the limits and the failure causes (open, syntax, full), but express them as V
  errors with useful messages.
- Keep the parsing rules: comments, blank lines, trimming, last duplicate wins, a value
  may contain `=`.
- Port every test case and add one for an empty file.
- Write `PORTING.md` with the API map and any deliberate difference.
- `v fmt -verify`, `v -check`, `v vet -W`, and `v test` must pass.

Input files: config.c, config.h, test_config.c
