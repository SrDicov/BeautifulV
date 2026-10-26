---
type: llm
focus: trace
weight: 1
---

REVIEW.md flags the `pub mut:` fields on `Item` and `Inventory` as an API problem:
callers can set any quantity, price, or item map without validation, so the type cannot
protect its invariants. It suggests private fields plus methods or a validating
constructor/factory instead of exposing the map and fields directly.
