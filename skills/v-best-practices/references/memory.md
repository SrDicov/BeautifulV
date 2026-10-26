# Memory, safety, and C interop

Source: `doc/docs.md` (Memory management, Memory-unsafe code, V and C), `doc/ownership.md`,
and the compiler's allocation tests.

## The default

Boehm tracing GC. Unreachable memory is freed; cycles are collected; you do not call
`free`. This is the correct choice for almost all code.

## Modes

| Mode | What it does | Use when |
|---|---|---|
| default | tracing GC | almost always |
| `-gc none` | no collector; you free | short-lived batch job; you can prove lifetimes |
| `-prealloc` | arena, freed at exit | short-lived, single-threaded batch |
| `-autofree` | compiler inserts frees (WIP) | do not depend on it yet |
| `-ownership` | static move checking | you want use-after-move at compile time |
| `-race` | ThreadSanitizer | testing concurrency; incompatible with `-gc none`/`-prealloc` |

- `-gc none` has no cycle collection and no finalisers. A long-running program that
  allocates will grow without bound.
- `-prealloc` is documented as suitable only for short-lived, single-threaded programs.
  It is not a server flag.
- Scoped arenas: `import arena`, `a := arena.new()`, `a.push()`, `a.pop()`, `a.reset()`,
  `a.free()`. Works with the default GC, not with `-prealloc`.

## Stack and heap

- V keeps a value on the stack when its address does not escape.
- Returning a reference to a local forces the local to the heap.
- `@[heap]` forces every value of a struct to the heap. Use it when a reference escapes
  through a field and the compiler cannot prove the lifetime.
- `&T{...}` allocates one struct on the heap and gives a `&T`.
- Prefer returning a value over returning `&Local`. The compiler can usually keep a
  value on the stack only if you do not take its address.

## Ownership checking

`-ownership` tracks owned strings and values with the ownership markers. It catches:

```v ignore
fn main() {
	s1 := 'hello'.to_owned()
	s2 := s1       // move
	println(s1)    // error: use of moved value
}
```

It also defines `$if ownership ? {}` and `_d_ownership.v` files. Strings created with
`.to_owned()`/`.clone()` and string slices participate, and ownership is inferred
recursively for aggregates: a struct with a string, array, map, interface, result, or
sum-type field is owning even without a marker. `Owned` forces move tracking on an
otherwise copyable struct; `Copy` permits copying and `Drop` enables a custom `drop()`.
Read `doc/ownership.md` before relying on it.

## unsafe

`unsafe` is a promise, not an escape hatch. It is required for:

- pointer arithmetic and pointer indexing
- converting between incompatible pointer types
- calling C functions that V marks unsafe
- accessing a union member
- `goto`

Rules:

- One small `unsafe` block per operation, with a comment saying why it is sound.
- Keep safe expressions outside the block so the checker can still see them.
- `@[unsafe]` on a function moves the requirement to the caller but still checks the
  body.
- `nil` references are written `unsafe { nil }`; prefer `?&T`.
- A null `voidptr` from C is still a null pointer. Check it before dereferencing.

## C interop

- Put `C.` declarations in `.c.v` files, not in ordinary `.v` files.
- `#include <header.h>` adds the include to the generated C; it does **not** teach V the
  declarations. Redeclare what you call.
- `#flag -I...`, `#flag -L...`, `#flag -l...`, `#pkgconfig lib` control the build.
- `@[typedef]` for a C `typedef struct`; `@[c_extern] __global` for an external global.
- C function parameter names may be uppercase. Use `const_name &char` to express
  `const char *` and survive `-cstrict`.
- C strings: `c'text'` creates one; `cstring_to_vstring(p)` copies a C string into V.
  A V string is **not** guaranteed to be NUL-terminated, so do not hand `.str` to C as a
  C string; use `c'...'` or copy it explicitly.
- A V pointer passed to C must outlive the call. C must not keep it after the V object
  is collected. `@[keep_args_alive]` covers the call-only case.
- `-Wimpure-v` reports a `C.` use that escaped a `.c.v` file.
- `-cstrict` makes the C compiler strict; it is the right CI companion for interop code.

## Allocation contracts

- `@[noalloc]` checks that a function's reachable code does not allocate; growing an
  existing mutable array parameter is the one exception.
- `@[noalloc: strict]` also forbids that growth.
- `@[manualfree]` tells the autofree engine to leave a function alone. It has nothing to
  do with the GC.
- `@[keep_args_alive]` is for C interop only.
