---
type: llm
focus: trace
weight: 1
---

`Item` and `Inventory` no longer expose `pub mut:` maps/fields that callers can corrupt.
Fields are private, access is through methods, and construction validates quantity and
price (or the type cannot be built in an invalid state).
