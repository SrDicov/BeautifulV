---
type: llm
focus: trace
weight: 1
---

`v fmt -verify .` and `v vet -W .` pass (or the agent reports the exact remaining
failure), and no public function is missing its doc comment if the project is a library.
