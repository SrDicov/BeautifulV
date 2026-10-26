---
tags: [v-port]
max_turns: 30
timeout_seconds: 1200
---

Port `taskgraph.py` to V. Port `test_taskgraph.py` first, then implement until every case
passes.

Requirements:

- Map `TaskError` to a V error or a typed error enum; no `panic` for a user error.
- `Kind` becomes a V enum; `priority: Optional[int]` becomes `?int`.
- `TaskGraph.add` keeps definition order. `order()` returns dependencies before
  dependents and rejects an unknown dependency or a cycle with an error.
- `summary()` groups by kind in the same order (`build`, `test`, `deploy`), keeps
  definition order inside a group, and prints the priority only when present.
- Port every test, including the cycle, unknown dependency, duplicate, and ordering
  cases.
- Write `PORTING.md` with the mapping and deliberate differences.
- `v fmt -verify`, `v -check`, `v vet -W`, and `v test` must pass.

Input files: taskgraph.py, test_taskgraph.py
