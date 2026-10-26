---
name: v-best-practices
description: >-
  Write fast, light, safe, idiomatic V. Use whenever writing, reviewing, refactoring,
  debugging, or explaining V code: modules, structs, sum types, options and results,
  generics, concurrency, memory and GC, C interop, tests, or v.mod. Trigger on any
  mention of V, vlang, .v or .vsh files, `v run`, `v fmt`, `v vet`, `v test`, `v -prod`,
  vlib, or v.mod, even when the user does not ask for "best practices" or "idiomatic"
  code. Synthesized from the V language reference, the compiler and tooling source, and
  the standard library.
license: MIT
---

# V best practices

V is small on purpose. The compiler is fast, the formatter is the style guide, and the
checker is the specification. Write code that is **fast** (values and fixed arrays before
allocation), **light** (no abstraction you cannot justify), and **safe** (options and
results instead of sentinels, `unsafe` only at a boundary).

Deeper material is in `references/`:

- `references/language.md` — options, results, sum types, mutability, comptime, conversions
- `references/memory.md` — GC modes, ownership, `unsafe`, C interop
- `references/performance.md` — flags, attributes, allocation, profiling
- `references/toolchain.md` — `v fmt`, `v vet`, `v -check`, `v test`, `v.mod`
- `references/ecosystem.md` — modules, dependencies, project layout, docs

Where this file and the compiler disagree, the compiler wins. Check the version with
`v version`.

## 1. Mindset

- **Write V-shaped code.** V has no classes, constructors, inheritance, exceptions,
  default arguments, or operator overloading beyond a small fixed set. Do not emulate
  them with proxies. Model data with structs and sum types, failure with `!T`, absence
  with `?T`, and behaviour with methods and interfaces.
- **Simplicity is the feature.** V's selling point is a small language that compiles in
  milliseconds. A pattern from Java, Rust, or TypeScript that needs three helper types
  to express is usually the wrong pattern here.
- **Make invalid states unrepresentable.** Use an enum instead of a `string` mode, a sum
  type instead of "only one of these fields is set", `?T`/`!T` instead of `-1`/`""`/`nil`,
  and `@[noinit]` plus a validating factory instead of a public struct anyone can build.
- **Application code is not library code.** A library (anything imported by another
  module) keeps fields private, documents every public function, and propagates errors.
  An application may be more direct, but still must not panic on user input.
- **Let the tools work.** `v fmt` owns layout. The compiler owns types and mutability.
  `v vet` owns public documentation and a few suspicious constructs. Spend your
  attention on data flow and failure modes.

## 2. Before you hand back code

Run these in order; each is cheaper than the next:

```bash
v fmt -w path/to/file.v
v fmt -verify path/to/file.v     # non-zero if the formatter would change it
v -check path/to/file.v          # types only, no binary
v vet -W path/to/file.v          # warnings as errors
v test path/to/dir/              # all _test.v files
v -prod -cstrict path/to/dir/    # release: all warnings fail, asserts removed
```

- For a **library** rather than a `main` module, `v -check` needs `-shared`
  (`v -check -shared path/`), or it fails with *"project must include a `main` module"*.
  `v vet` does not need `-shared`.
- `-prod` removes every `assert`. Never use `assert` as input validation or as the only
  guard before indexing; validate and return a result.
- `v vet` only checks **public** functions for documentation unless you pass `-p`.
- A formatting pass is not a compile check, and a green test run is not a formatting
  check. Run both.
- Do not silence a warning with `_ :=` unless the discard is genuinely intentional.
  `_ := os.write_file(...)` hides a failure; handle it.

## 3. Variables, mutability, ownership

- `:=` declares and initialises; `=` assigns. Every variable has a value from birth.
- Variables are immutable by default. `mut` is a promise that the value changes.
- Shadowing is a compile error. Use a new name, or restructure.
- Global variables are disabled by default. Do not reach for `-enable-globals` in
  application code; pass state explicitly or use a module-level `const`.
- Function arguments are immutable by default, even references. Mark `mut` at the
  declaration **and** at the call: `fn fill(mut a []int)` then `fill(mut items)`.
- The receiver rule is the one that bites. A `mut` receiver mutates the caller and
  requires the caller's variable to be `mut`; a plain receiver cannot mutate, and
  `mut` on a `&T` receiver is rejected by the compiler:

```v
struct Counter {
mut:
	n int
}

fn (mut c Counter) bump() {
	c.n++ // changes the caller's value
}

fn (c &Counter) value() int {
	return c.n // read-only; & avoids copying a large struct
}

fn main() {
	mut c := Counter{}
	c.bump()
	println(c.value())
}
```

- To mutate, write `fn (mut c Counter)`, not `fn (mut c &Counter)`. Use `fn (c &T)`
  when a read-only method should avoid copying a large value. The call site must say
  `mut` when the receiver is mutable, so the change is visible in the signature.
- Prefer returning a new value over mutating an argument. Mutating arguments is for
  measured hot paths that need to avoid a copy.
- Primitive arguments cannot be mutated; only larger types such as arrays, maps, and
  structs.
- Closures capture **copies**, and must list what they capture: `fn [x] () { ... }`.
  Capture a reference (`&x`) when the closure must see later changes.
- `defer { cleanup() }` runs at scope exit, in reverse order. Use it for files, locks,
  temporary directories, and arena scopes.

## 4. Types and API design

### Naming

| Item | Case | Example |
|---|---|---|
| Types, sum types, interfaces | `PascalCase` | `HttpClient`, `ParseError` |
| Functions, methods, locals, fields | `snake_case` | `read_line`, `user_id` |
| Constants | `snake_case` | `max_retries` |
| Modules and directories | `snake_case`, under 10 chars | `net.http`, `json2` |
| Enum variants | `snake_case` | `.pending`, `.in_progress` |
| Receivers | one short lowercase name | `fn (u User) name() string` |

V has no acronym convention beyond `PascalCase`: write `HttpClient`, not `HTTPClient`.
Files are `snake_case.v`; a test is `foo_test.v` beside `foo.v`.

### Structs

- Fields are **private and immutable** by default. That is the right default for a
  library. Expose `pub:` for data that is safe to read anywhere; use `pub mut:` only
  when external mutation cannot break an invariant.
- Avoid `__global:` entirely. It makes a field mutable outside the module.
- Use defaults for genuinely optional state, `@[required]` for fields a caller must set,
  and `@[noinit]` plus a factory when construction must validate:

```v
@[noinit]
pub struct Port {
pub:
	number int
}

pub fn new_port(number int) !Port {
	if number < 1 || number > 65535 {
		return error('port ${number} is out of range')
	}
	return Port{
		number: number
	}
}
```

- Static type methods group constructors and helpers on the type:
  `fn User.new(name string) User` is called as `User.new('ana')`. Prefer it to a loose
  `new_user` function in a library.
- `@[params]` on a struct used as a trailing argument gives named, optional call syntax:

```v
@[params]
pub struct SearchOptions {
	limit int = 20
	exact bool
}

pub fn search(text string, opts SearchOptions) []string {
	// ...
	return [text]
}

_ := search('v', limit: 5)
```

- Embedding is composition, not inheritance. You cannot cast an embedding struct to its
  embedded field, and duplicate names need an explicit `outer.Inner.name`.
- Use a union only for C layout or a measured memory win. Access to a union member is
  `unsafe`; a sum type is usually the safe model.
- `@[heap]` only when a reference must outlive the stack frame. It usually signals a
  design that returns `&T` from a local; prefer returning a value.

### Enums and sum types

- Enums are integer-backed. Give them an explicit backing type when the size or C ABI
  matters: `enum Kind as u8`.
- `@[flag]` turns an enum into a bitfield with `has`, `all`, `set`, `clear`, `toggle`.
- Match on an enum must be exhaustive or have `else`. Prefer exhaustive: adding a variant
  should break every incomplete match at compile time.
- `Enum.from(value)` returns `!Enum`; do not hand-roll string-to-enum maps.
- A sum type is a closed set: `type Node = File | Dir | Symlink`. Use `match` or
  `if x is File` and let the compiler smart-cast. `as` panics when the variant is wrong;
  prefer a checked branch.
- For recursive types, store references or dynamic arrays; a value field that contains
  itself is rejected because the layout would be infinite.
- Named variants (`type Expr = IntLit(int) | Void`) let two variants carry the same
  payload type. Use them when a bare type name would be ambiguous.

### Interfaces and generics

- V interfaces can declare fields and methods, and implementation is implicit. Add the
  optional `implements I` clause when the intent is not obvious.
- Do not add an interface for a single implementation. A concrete type plus a function
  is smaller and faster.
- Generic functions are monomorphised: a `[T]` function becomes one concrete function per
  type argument. Use generics when the algorithm is genuinely type-independent, not to
  imitate a framework.
- Constrain a type parameter with an interface, a sum type, or a struct when the body
  needs members that the constraint provides. Operators in an unconstrained generic are
  checked for each instantiation, so a body that compares `T` only works for types that
  support the comparison.
- Keep `voidptr` and `unsafe` out of public APIs. They erase the information the checker
  exists to provide.

## 5. Errors, options, and panics

- `?T` means a value or `none`; it carries no error. Use it for a lookup that is
  legitimately absent.
- `!T` means a value or an `IError`. Use it for anything that touches the outside world:
  files, network, parsing, validation.
- Handle with `or {}`, `!`, or `if v := expr {}`:

```v
import os
import strconv

fn read_port(path string) !int {
	text := os.read_file(path) or { return error('cannot read ${path}: ${err.msg()}') }
	return strconv.atoi(text.trim_space()) or {
		return error('bad port in ${path}: ${err.msg()}')
	}
}

fn main() {
	port := read_port('port.txt') or {
		eprintln(err.msg())
		exit(1)
	}
	println(port)
}
```

- On a **result**, `or { default }` silently converts a failure into a value. Only do
  that when the default is honest (a cache miss, an optional setting).
- `or { panic(err) }` belongs in a prototype or an unrecoverable entry point, never in
  library code and never on user input.
- `x!` propagates and immediately returns on failure. In `main` it panics, because there
  is nowhere left to propagate.
- Custom errors embed the built-in `Error` and override `msg()` and, when useful, `code()`.
  The embedded `Error` supplies the default implementations.

```v
pub struct ConfigError {
	Error
	path string
}

pub fn (e ConfigError) msg() string {
	return 'invalid config at ${e.path}'
}

pub fn (e ConfigError) code() int {
	return 2
}
```

- A function returning `!` or `?` with no payload is a result/option of `void`; use it
  for operations whose success is the whole answer.
- Only one option or result may appear in a multi-return. Wrap the rest in a struct.
- Never encode failure as `0`, `-1`, `""`, or a magic string. That is what the two
  prefixes are for.
- Panic only for a programming error: an impossible state, a violated invariant, or a
  failed assertion. A panic in a spawned thread takes down the process.

## 6. Idioms that keep V code small

- **Strings are immutable UTF-8.** Indexing yields a **byte**, `.len` is bytes, and
  `.runes()` is characters. Use `runes()` for user-visible character counts and
  `utf8` for slicing by code point.
- **Interpolate instead of concatenating.** `'${name} is ${age}'` is clearer and avoids
  repeated allocations. In a loop, use `strings.new_builder(cap)` and `write_string`.
- **Arrays.** `[1, 2, 3]`, `[]int{cap: n}`, append with `<<`, test membership with `in`,
  transform with `.filter`, `.map`, `.any`, `.all`, `.sorted()`. A slice is a view over
  the parent's buffer: with a mutable parent, writes through either are visible through
  the other until the parent reallocates. The exception is `mut s := immutable[1..]`,
  which the compiler clones so the immutable parent cannot change. Use `.clone()` for an
  independent copy; `unsafe { a[..] }` shares memory.
- **Fixed arrays** (`[4]f32`) live on the stack, have a constant length, and avoid an
  allocation. They are the right buffer type in a parser or a hot loop; convert with
  `[..]` when you need dynamic methods.
- **Maps preserve insertion order.** Use a map when order matters and you want O(1)
  lookup; do not add a separate key array "just in case".
- **A missing map key returns the zero value.** Check with `key in m` or `if v := m[key]`
  when zero is a valid value. Add `@[strict_map_index]` to the module to make unguarded
  indexing a compile error.
- **`match` is exhaustive over enums and sum types; ranges use `...` (inclusive).**
  Do not write a chain of `if` for a closed set.
- **`in [ ... ]` with one element is a vet error.** Write `x == 1`.
- **`defer` is the cleanup tool.** It runs after the return value is evaluated, so a
  deferred `close` can still see the result.
- **`@[must_use]`** warns when a call's return value is discarded. It is a warning, not
  an error, so it will not protect a fallible call by itself.
- **`dump(x)`** is a development tool. Remove it before committing.

## 7. Memory and safety

- **Default GC: do nothing.** The Boehm collector is the safe, boring choice. Most V
  code should never think about it.
- `-gc none` removes the collector. You must then free everything, cycles leak, and
  finalisers do not run. Use it for a short-lived batch job, not a server.
- `-prealloc` is an arena for short-lived, single-threaded, batch-like programs. Not a
  general performance switch.
- `-autofree` is still work in progress. Do not build a design on it.
- `-ownership` is opt-in static move checking. It tracks owned strings and recursively
  owning aggregates: a struct with a string, array, map, interface, result, or sum-type
  field is owning even without a marker.
- Values live on the stack when the compiler can prove the address does not escape.
  `@[heap]` forces heap allocation; `&T{}` allocates one value on the heap.
- `unsafe` is for C interop and a few low-level primitives. Keep the block as small as
  possible, put only the unsafe operation inside, and say why it is sound:

```v
fn clear_last(mut pixels []u8) {
	if pixels.len == 0 {
		return
	}
	// SAFETY: the length was checked above, so the index is in range.
	unsafe {
		pixels[pixels.len - 1] = 0
	}
}
```

- `@[unsafe]` on a function forces callers into an `unsafe` block but still checks the
  body, which is the right shape for a small primitive.
- A null reference is `unsafe { nil }`. Prefer `?&T` for a reference that may be absent.
- Put C declarations in `.c.v` files, not in ordinary `.v` files. Use `#include`,
  `#flag`, and `#pkgconfig` for the build; declare only what you call. `-Wimpure-v`
  catches a `C.` call that escaped the interop file.
- A V pointer passed to C must stay alive for the call and must not be retained by C
  after the V object dies. `@[keep_args_alive]` exists for the narrow case where C keeps
  a pointer only during the call.
- `@[noalloc]` proves a function does not allocate. It is a contract, not a suggestion:
  a hidden allocation becomes a compile error. Use it on a hot path after measuring.

## 8. Concurrency

Reach for the highest-level tool that solves the problem:

| Situation | Use |
|---|---|
| Process a slice, collect results | `parallel.amap(items, fn (x T) R { ... })` |
| Process a slice, discard results | `parallel.run(items, fn (x T) { ... })` |
| A fixed set of independent tasks | `spawn f()` plus the returned thread handle |
| Many tasks of the same type | `[]thread T`, then `threads.wait()` |
| Pass work between threads | `chan T` |
| Share mutable state | `shared` plus `lock`/`rlock` |
| One counter or flag | `sync.stdatomic` |

- **`spawn` returns a handle. Keep it and call `wait()`.** A discarded handle detaches the
  thread, and the program or function may exit while it is still writing.
- `sync.WaitGroup` exists, but `parallel.amap`/`run` and the thread-array `wait()` are
  harder to get wrong.
- **Never let a spawned function panic.** It terminates the process.
- A `shared` struct has a hidden lock. Use `lock x { ... }` to write and `rlock x { ... }`
  to read; keep the critical section short and never hold it across a channel operation
  or another lock.
- A `shared` map locks each single operation. Two operations that must agree still need
  an explicit `lock` block, and iteration needs an `rlock`.
- Prefer channels for ownership transfer and `shared` for a small piece of state that
  many threads read. Do not build a lock-free algorithm unless you can test it under the
  race detector.
- `v -race run .` and `v -race test .` are the authority. A clean run does not prove the
  absence of races, but a report is real. `-race` cannot be combined with `-gc none` or
  `-prealloc`.

## 9. Performance, in order

1. **Measure.** `v -prod -prof prof.txt run .` profiles per function; `v -show-timings`
   reports compile phases. Use a profiler before changing an algorithm.
2. **Build with `-prod`.** It optimises the C and removes asserts. Do not benchmark a
   debug build.
3. **Remove allocations.** Preallocate arrays (`[]T{cap: n}`), reuse buffers, prefer
   fixed arrays for small stack buffers, and build strings with `strings.Builder`.
4. **Use values, not pointers.** A struct passed by value is usually cheaper than an
   allocation and an indirection. Return values instead of mutating an argument unless
   the copy is measured as the cost.
5. **Pick the right container.** A map is O(1) but allocates; a fixed array is a stack
   buffer; a slice is zero-copy but aliases its parent. Choose by the access pattern.
6. **Only then tune the compiler.** `@[inline]`, `@[direct_array_access]`, `-fast-math`,
   `-cflags -march=native`, and PGO are last resorts with real costs.
   `@[direct_array_access]` removes bounds checks, so it is only acceptable with a proof
   of the index range.

```v
// Bounds are guaranteed by the loop, so the access check is redundant.
@[direct_array_access]
fn sum(xs []int) int {
	mut total := 0
	for i in 0 .. xs.len {
		total += xs[i]
	}
	return total
}
```

- `-gc none` is not a performance switch; it is a memory-management decision.
- `@[packed]` and `@[minify]` trade layout and reflection for size. Do not apply them to
  a struct that is serialised.
- `_likely_` / `_unlikely_` only when the branch is genuinely one-sided.

## 10. Modules, projects, and dependencies

- A module is a directory; every `.v` file in it declares `module <directory>`.
  The top-level project directory is special: files may declare different modules there.
- Module names are short, `snake_case`, and match the directory. Import cycles are not
  allowed. A symbol is reached with a **single** module prefix, whatever the nesting:
  `abc/def/source.v` declares `module def` and is called as `def.func()`.
- A project has one `v.mod` at its root. `v new` and `v init` create it. The folder
  containing it is the module-lookup root; `name`, `dependencies`, and `dev_dependencies`
  are the fields VPM reads. Add every dependency you import.
- `v install <module>` installs into `~/.vmodules` (or `$VMODULES`); a project install
  resolves the manifest and writes `v.mod.lock` beside it. Commit the lockfile, and use
  `v install --locked` in CI so a stale lock fails instead of resolving silently.
  `--frozen` also refuses to rewrite it.
- Dependency strings may be bare names, exact tags, branches, URLs, or semver ranges
  (`lib@^1.2.0`, `lib@>=0.4.0 <0.6.0`); a bare name follows the default branch.
- `v link` symlinks the current project into `~/.vmodules` while you develop it;
  `v unlink` before publishing.
- Use `v why <module>` for the requirement chain with constraints, `v mod why <module>`
  for the import chain, and `v list`, `v outdated`, `v update`, `v upgrade` to maintain
  dependencies.
- A `.vsh` file is a script: `os` is imported and its functions are unqualified. Use it
  for a one-off task; move logic into a normal module with tests when it grows.

## 11. Documentation and tests

- Every public declaration gets a comment immediately above it, starting with its name:
  `// parse_header parses ...`. `v vet -W` enforces this for public functions.
- Write in the present tense and document the failure modes of a `!T` function: what it
  returns on failure and why.
- `v doc .` generates documentation; a module overview goes in the first comment after
  the `module` line.
- Tests are `test_` functions in `_test.v` files. They take no parameters and use the
  builtin `assert`. Put the **got** value before the **want** value and add a message:

```v
import strconv

fn parse_port(s string) !int {
	return strconv.atoi(s)
}

fn test_parse_port() {
	got := parse_port('8080') or { -1 }
	assert got == 8080, 'parse_port returned ${got}, want 8080'
}
```

- Test options/results with `assert f() == none` and check the error message when the
  message is part of the contract.
- Fixtures go in `os.vtmp_dir()` with a `defer` cleanup. `testdata/` is ignored by the
  test runner and is the place for invalid source or input files.
- `testsuite_begin` and `testsuite_end` run around a file's tests.
- Run a subset with `VTEST_ONLY='pattern' v test dir/` or
  `VTEST_ONLY_FN='test_name' v test dir/`. `v -silent test` hides passing tests;
  `v -stats test` adds timings.
- Coverage: `v -no-skip-unused -coverage vcov/ test .`, then `v cover vcov/`. Without
  `-no-skip-unused`, uncalled functions are removed from the build and cannot be counted.
- Break a new test once, watch it fail for the right reason, then put it back.

## 12. Final checklist

- [ ] `v fmt -verify` passes on every touched file.
- [ ] `v -check` passes (`-shared` for a library).
- [ ] `v vet -W` passes; public functions have doc comments that start with their name.
- [ ] `v test` passes; new behaviour has a test that has been seen to fail.
- [ ] `v -prod -cstrict` builds cleanly and does not rely on removed asserts.
- [ ] Errors propagate; no `or { panic }`, `or { 0 }`, or magic sentinel on a fallible path.
- [ ] `unsafe` is minimal, at a boundary, and has a reason.
- [ ] Every spawned thread is waited for; shared state is locked; `-race` is clean.
- [ ] Public structs keep invariants private; construction goes through a validating
      factory where needed.
- [ ] No `dump`, no commented-out code, no unused imports or variables.
