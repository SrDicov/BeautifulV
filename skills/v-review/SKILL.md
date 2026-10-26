---
name: v-review
description: >-
  Review existing V code for correctness, idiom, API design, error handling, memory
  safety, concurrency, and performance, and produce a prioritized report where every
  finding cites a file and line, the rule it breaks, and a concrete fix. Use whenever
  the user asks to review, audit, critique, grade, or "check" V code, a module, a .v
  file, a diff, or a pull request, or asks what is wrong with their V and how to make it
  more idiomatic, even if they do not say "review". Pairs with v-best-practices for the
  rules themselves.
license: MIT
---

# Reviewing V code

A V review is not a style opinion. The compiler, the formatter, the vetter, and the race
detector already decide most mechanical questions. Your job is the layer above them:
failure modes, data flow, ownership, API shape, and the places where a green build still
hides a bug.

## 1. Read before you judge

1. Read the whole target, its `v.mod`, and its `_test.v` files. A function without its
   callers and tests is not enough context.
2. Load the `v-best-practices` skill for the rule set. This file names the checks; that
   file explains them.
3. Run what the environment allows, from the project root:

```bash
v version
v fmt -verify .
v -check .                 # -shared for a library
v vet -W .
v test .
v -prod -cstrict .         # if the project builds cleanly
```

If a command cannot run (no `v`, missing C compiler, missing service), say so in the
report. Do not pretend a check ran.

4. **Do not change the code unless the user asked for a fix.** A review produces a
   report. If you need to prove a failure, copy the file to a temporary directory and
   experiment there.

## 2. What to look for

### Correctness and failure

- **`or { panic(...) }` on a fallible path.** A library that panics on a missing file or
  a bad number is a denial of service. Propagate `!T`.
- **`or { 0 }`, `or { '' }`, `or { false }` on a result.** The default silently turns a
  failure into data. Only a genuinely optional value may take a default.
- **`_ :=` on an operation that can fail.** The error is gone. Handle it or propagate it.
- **`.int()` / `.f64()` on untrusted text.** These parse without a result; use
  `strconv.atoi`/`strconv.parse_int` and handle the error.
- **A `!` in `main`.** It panics with a bare error. That may be acceptable for a CLI, but
  the message and exit code should be deliberate.
- **Errors without context.** `return err` hides which file, key, or record failed.
  Wrap with `error('reading ${path}: ${err.msg()}')`.
- **A custom error without `msg()` or with a meaningless `code()`.**
- **A sentinel (`-1`, `0`, `""`) where `?T` belongs.**

### Mutability and ownership

- **A mutation attempt on a read-only receiver.** `fn (s &S)` cannot change a field;
  the method needs `fn (mut s S)`, and the caller needs a `mut` variable.
- **`fn (mut s &S)`.** The compiler rejects it; use `fn (mut s S)` for mutation and
  `fn (s &S)` for a read-only method on a large struct.
- **A `mut` argument that is never mutated**, or a copy made only to satisfy `mut`.
- **A changed value discarded**: a method that returns the updated value, called without
  assigning or using the result.
- **A closure capturing by value when it needs the outer change.** It must capture a
  reference.
- **`@[heap]` used to silence a lifetime error** without understanding why the reference
  escapes.

### Types and API

- **Public mutable fields (`pub mut:`) on a type with invariants.** Anyone can break it.
- **A public struct with no validating constructor**, or `@[noinit]` missing where the
  fields must stay consistent.
- **A `bool` parameter that selects a mode.** Use an enum or two functions; the call site
  `render(true)` is unreadable.
- **A `string` used as a mode or a state machine.** Use an enum, and `Enum.from` at the
  boundary.
- **`voidptr` or `unsafe` in a public signature.** The type system stops helping.
- **An interface with one implementation**, or a generic function that is only ever
  called with one type. Both add indirection for nothing.
- **A sum type with an `else` branch** that silently accepts a new variant. Prefer an
  exhaustive match.
- **`as` on a sum type** where `is`/`match` would avoid a panic.
- **A map used as a record** with string keys and unchecked lookups.

### Concurrency

- **A discarded `spawn` handle.** The thread is detached; the function can return or the
  process can exit while it writes.
- **A `WaitGroup` with `done()` but no matching `add()`**, or `add` after `wait`.
- **`shared` state read or written outside `lock`/`rlock`**, or two operations that must
  be atomic split across locks.
- **A lock held across a channel send, a `spawn`, or another lock.** Deadlock or
  priority inversion.
- **A spawned function that can panic.** It takes the process down.
- **`spawn` in a loop over an unbounded collection.** Use a worker pool or
  `parallel.amap`.
- **`-race` never run** on concurrent code.

### Memory and safety

- **`unsafe` without a comment, or a large block containing safe code.** Shrink it and
  say why it is sound.
- **`-gc none` in a long-running program** with no free strategy, or with cycles.
- **A V pointer handed to C that C may retain**, or a C pointer dereferenced without a
  null check.
- **`@[direct_array_access]` without a proof of the index range.**
- **`@[packed]` / `@[minify]` on a struct that is serialised or reflected.**

### Performance (only with evidence)

- **`clone()` inside a loop**, repeated `s += x`, or a dynamic array with no `cap` on a
  known-size build.
- **A pointer or interface where a value would do**, when the code is hot.
- **`-gc none` or `-prealloc` used as a performance switch** without measurement.
- Flag micro-optimisations as **consider**, not must-fix. A review that demands
  `@[inline]` without a profile is noise.

### Tests and tooling

- **No test for a fallible path**, or a test that cannot fail.
- **`assert` without a message**, so a failure does not show got versus want.
- **Tests that depend on wall-clock time, network, or a fixed path.**
- **Formatting or vet failures.** These are must-fix because CI will reject them.

## 3. Rank findings

Use exactly three tiers:

| Tier | Meaning |
|---|---|
| **Must fix** | Wrong result, panic on input, data race, memory unsafety, a leaking error, or a build/format/vet failure. |
| **Should fix** | API shape that will be hard to change later, a missing test on a contract, a misleading name, a real but non-critical inefficiency. |
| **Consider** | Style, naming, micro-performance, a simpler structure. |

Order by severity, then by how many call sites a fix touches. Group repeated instances
of the same mistake under one finding and list every location.

## 4. Write the report

Use this shape:

```markdown
# Review: <target>

## Verdict
<One paragraph: is this safe to build on? What is the single most important problem?>

## Must fix
### 1. <short title>
`path/file.v:42` — <rule and why it matters>.
```v ignore
// current
...
// fix
...
```

## Should fix
...

## Consider
...

## What is already good
- <specific thing the code does well, with a location>
```

Rules for each finding:

- Give a **`file:line`** location, not "somewhere in the parser".
- Name the rule and its source: `?T` vs `!T`, mutability, module naming, `v vet`
  documentation, a V documentation section, or a language guarantee.
- Show the **smallest** code change that fixes it. Do not paste the whole file.
- Say what breaks if the code stays as it is.
- If a finding depends on runtime context you cannot see (traffic, allocation profile),
  say so and mark it **consider**.

End with what the code already does well. It is not flattery; it tells the author which
patterns to keep.

## 5. Review checklist

- [ ] `v fmt -verify` and `v vet -W` were run, or the report says why not.
- [ ] `v -check` was run (`-shared` for a library), or the report says why not.
- [ ] `v test` was run, or the report says why not.
- [ ] Every fallible path has a handled error.
- [ ] Every `spawn` is waited for, every `shared` access is locked.
- [ ] Every `unsafe` block has a reason and is minimal.
- [ ] Mutating receivers reach the caller.
- [ ] Public types keep their invariants private.
- [ ] New behaviour has a test that could fail.
- [ ] The report names what is good, and does not invent problems to fill space.
