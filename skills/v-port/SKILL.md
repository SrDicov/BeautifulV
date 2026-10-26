---
name: v-port
description: >-
  Port C, C++, Python, Go, JavaScript, TypeScript, or Rust to idiomatic V while keeping the
  original behaviour: options and results instead of null and exceptions, sum types and
  enums instead of flag integers and magic strings, slices and maps instead of manual
  arrays and dictionaries, and the original tests ported first so the two versions can
  be proved equivalent. Use whenever the user asks to port, translate, convert, migrate,
  or rewrite code in V ("rewrite this in V", "what would this look like in V", "move this
  module to V"), even if they do not say "port". Pairs with v-best-practices for the
  target style.
license: MIT
---

# Porting to V

A port is not a transliteration. The original tells you what the code must do; V tells
you how to say it. Keep the behaviour, replace the scaffolding.

## 1. Inventory the original

Before writing V:

- Build and run the original test suite. Record the command and the result.
- Find the real entry points, the public API, and every error path.
- List the data structures and their invariants: what may be null, what may fail, what
  order matters, what is mutated, what is shared.
- Note the platform and library dependencies that must become `v.mod` dependencies or a
  `.c.v` interop layer.
- Identify undefined behaviour and bugs. Do not port them silently; list them in
  `PORTING.md` and choose a defined behaviour.

## 2. Port the tests first

Write the V tests before the implementation. A test that compiles against a stub is the
specification.

- `foo_test.v` beside `foo.v`; `fn test_...()` takes no arguments; `assert` is builtin.
- Assert **got before want**, with a message: `assert got == want, 'got ${got}'`.
- Port the original test cases one by one, including the edge cases and the error cases.
- Replace exception assertions with result assertions: `assert f() == none` or
  `if f() { assert false }`.
- Use `os.vtmp_dir()` for fixtures and `defer` for cleanup.
- Keep the original expected output byte-for-byte if it is part of the contract.

## 3. Design the V API

Do not preserve the original's shape when the language no longer needs it.

- **Errors** become `!T`; absence becomes `?T`. A function that returned `-1`, `NULL`, or
  threw an exception gets one of these two prefixes.
- **Out-parameters** become return values or a small result struct. V returns multiple
  values: `fn parse(s string) !(Node, int)`.
- **Flag integers** become `@[flag]` enums; **string modes** become enums; **boolean
  parameters** become an enum or two functions.
- **Classes with a single implementation** become a struct with methods. Inheritance
  becomes embedding plus an interface only where polymorphism is real.
- **A dictionary whose order matters** becomes a V map. V maps preserve insertion order,
  so do not add a parallel key array unless the original relied on a different order.
- **Tuples** become a named struct when they cross a module boundary; keep multi-return
  for local, obvious pairs.
- **`kwargs` / optional arguments** become an `@[params]` struct.
- **Destructors / `with` / `finally`** become `defer`.
- **A nullable handle** becomes `?&T`; a handle that must be valid becomes `&T` and a
  constructor that validates.
- **A public mutable record** keeps its fields private and exposes methods or a
  validating factory.

Keep the original names only when they are part of the public contract. Otherwise use V
naming: `PascalCase` types, `snake_case` functions, fields, and constants.

## 4. Language mapping

### C

| C | V |
|---|---|
| `NULL` pointer | `?&T`, or `!T` when null is an error |
| error code / `errno` | `!T` with a custom error; `or {}` at the boundary |
| `malloc`/`free` | GC by default; `-gc none` only with a free strategy |
| `T *buf, size_t len` | `[]T` or `&T` + explicit length |
| `char *` | `string` (copy at the boundary); `c'...'` for a C call |
| fixed buffer `T[N]` | `[N]T` on the stack, or a slice |
| `struct` | `struct`, fields private by default |
| `union` | a sum type; a union only for C layout, with `unsafe` access |
| function pointer | `type Callback = fn (T) R` |
| `goto` | `for`, labelled `break`/`continue`, or a small state machine |
| macro constant | `const` |
| macro function | `@[inline] fn`, `$if`, or a generic |
| varargs `...` | `...T` for a V function; `...voidptr` only in a `C.` declaration |
| C header | a `.c.v` file with `C.` declarations, `#include`, `#flag` |

### C++

| C++ | V |
|---|---|
| class + constructor | struct + `fn Type.new(...) !Type` |
| inheritance | embedding + interface |
| virtual method | interface method |
| template | generic `[T]` |
| `std::vector<T>` | `[]T` |
| `std::map` / `std::unordered_map` | `map[K]V` (insertion ordered) |
| `std::string` | `string` |
| `std::unique_ptr<T>` | `T` by value, or `&T` |
| `std::shared_ptr<T>` | `shared T`, or GC-managed `&T` |
| RAII | `defer` |
| exception | `!T` |
| `operator==` | a normal method; V has limited operator overloading |
| namespace | module |

### Python

| Python | V |
|---|---|
| `None` | `?T` |
| `raise` / `try` | `!T` / `or {}` |
| `list` | `[]T` |
| `dict` (ordered) | `map[K]V` (insertion ordered) |
| `set` | `map[T]bool` |
| `tuple` | struct, or multi-return |
| `@dataclass` | struct with defaults |
| `Enum` | enum |
| `**kwargs` | `@[params]` struct |
| duck typing | interface |
| list comprehension | `.filter()` / `.map()` |
| `with` | `defer` |
| generator | struct with `next() ?T` |
| arbitrary-size `int` | `int` / `i64` / `i128`; check the range |
| `str` (code points) | `string` (UTF-8 bytes); use `.runes()` for characters |
| threads / GIL | `spawn`, `shared`, channels; no GIL |
| `asyncio` | threads and channels, or an event loop over `net` |

### Go

| Go | V |
|---|---|
| `error` | `!T` |
| `nil` | `?T` |
| goroutine | `spawn`, `parallel.amap` |
| channel | `chan T` |
| `defer` | `defer` |
| `interface{}` / `any` | sum type or generic, not `voidptr` |
| struct tags | `@[json: name]` and other attributes |
| `sync.Mutex` | `shared` + `lock`, or `sync.Mutex` |
| `context.Context` | an explicit parameter or a cancellation channel |
| `fmt.Sprintf` | string interpolation |
| `map` | `map[K]V`, insertion ordered |

### JavaScript / TypeScript

| JS/TS | V |
|---|---|
| `undefined` / `null` | `?T` |
| `throw` | `!T` |
| object literal | struct |
| `Array` | `[]T` |
| `Map` | `map[K]V` (insertion ordered) |
| `class` | struct + methods |
| union type | sum type |
| `any` | generic or a sum type; do not use `voidptr` to avoid the checker |
| `Promise` / `async` | a thread, channel, or event loop; V's async support is not the default |
| `JSON.parse` | `json2.decode` with a `[T]` type argument, returning `!T` |
| `JSON.stringify` | `json2.encode(value)` |

### Rust

| Rust | V |
|---|---|
| `Option<T>` | `?T` |
| `Result<T, E>` | `!T` |
| trait | interface |
| enum with data | sum type |
| `match` | `match` (exhaustive) |
| `Vec<T>` | `[]T` |
| `HashMap` | `map[K]V` (insertion ordered) |
| `Rc` / `Arc` | GC-managed reference, or `shared` for mutation |
| `RefCell` / `Mutex` | `shared` + `lock`, or `sync.Mutex` |
| ownership / borrow checker | GC by default; `-ownership` for opt-in move checks |
| `Drop` | `defer`, `cleanup()`, or a `free()` method |
| macro | comptime `$` or a generic function |
| `async` / `await` | threads and channels; do not transliterate the runtime |

## 5. Strings, numbers, and ordering

These are the differences that pass a smoke test and fail in production:

- **Strings are UTF-8 bytes.** `s.len` is bytes, `s[0]` is a `u8`. Python's `len` and
  JavaScript's `s.length` count UTF-16 code units; both differ from V for non-ASCII
  input. Use `.runes()` when the contract is characters.
- **Integer width.** V's `int` is 32 or 64 bits depending on the target; Python's is
  arbitrary. Pick `i64`/`i128` explicitly at a boundary and test overflow. V wraps by
  default; `-check-overflow` turns that into a runtime panic.
- **Float formatting** differs between languages. If the output is part of the contract,
  assert the exact string and use a format specifier.
- **Map order.** V preserves insertion order. If the original sorted keys, keep sorting;
  if it relied on hash order, make the new order explicit.
- **Error text.** If callers parse it, preserve it. Otherwise improve it and record the
  change in `PORTING.md`.
- **Exit codes and stdout/stderr.** A CLI port must keep them. Test with a subprocess.

## 6. C interop, when the port needs it

Prefer a pure V implementation. When the original is a C library:

- Put declarations in `foo.c.v`; keep ordinary `.v` files free of `C.`.
- Declare only the functions and fields you use; a C struct may be partially declared.
- `@[typedef]` for `typedef struct`; `@[c_extern] __global` for an external global.
- Use `c'...'` for a C string literal and `cstring_to_vstring` to copy a returned
  `char*` into a V string.
- A pointer passed to C must outlive the call; C must not retain it after the V object
  dies. `@[keep_args_alive]` is for the call-only case.
- Build flags go in `#flag`/`#pkgconfig`, not in a shell script that only works on your
  machine.
- Test with `-cstrict` and, if available, a sanitizer build.

## 7. Write `PORTING.md`

Every port ships a `PORTING.md` next to the code:

```markdown
# Porting notes: <module>

## Source
<project, commit, licence>

## API map
| Original | V | Notes |
|---|---|---|
| `parse_config(path)` | `Config.parse(path) !Config` | error instead of NULL |

## Tests
- `test_config.v` ports all 14 cases from `test_config.c`.
- Two new cases cover the empty file and a missing key.

## Deliberate differences
- Errors carry the path and line; the original returned a generic code.
- `int` is `i64` because the original used 64-bit counters.
- Map iteration is insertion-ordered; the original's order was unspecified.

## Not ported
- `debug_dump()`; it depended on an internal C struct.
```

## 8. Verify

Run, in order:

```bash
v fmt -verify .
v -check .                  # -shared for a library
v vet -W .
v test .
v -prod -cstrict .
```

Then verify behaviour against the original, not just the tests:

- Run both implementations on the same inputs and diff stdout, stderr, and exit codes.
- Exercise the error paths, empty inputs, and the largest realistic input.
- If the code is concurrent, run `v -race test .`.
- If it is a library, write one external test that imports it the way a user would.

Report what you ran and what still differs. A port without a passing original test suite
is not finished.

## 9. Porting checklist

- [ ] The original builds and its tests pass before the port starts.
- [ ] Every original test is ported, plus the edge cases the original missed.
- [ ] The V API is designed, not transliterated.
- [ ] Errors are `!T`, absence is `?T`; no sentinels.
- [ ] Names follow V conventions unless the public contract says otherwise.
- [ ] String byte/rune, integer width, and map order are handled.
- [ ] C interop is isolated in `.c.v` and passes `-cstrict`.
- [ ] `PORTING.md` records the API map and every deliberate difference.
- [ ] `v fmt -verify`, `v -check`, `v vet -W`, `v test`, `v -prod -cstrict` pass.
