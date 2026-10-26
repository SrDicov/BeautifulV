---
type: llm
focus: trace
weight: 1
---

REVIEW.md flags `find` returning `Item` from `inv.items[name]`. It explains that a
missing key returns a zero `Item`, which is indistinguishable from a real item named
`''` with quantity 0 and price 0, and suggests `?Item` or `(Item, bool)`.
