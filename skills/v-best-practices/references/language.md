# V language rules, condensed

Source: `doc/docs.md` in the V repository, plus the compiler's tests. When this file
and the compiler disagree, the compiler is right.

## Option and Result

| Type | Holds | Unwrap |
|---|---|---|
| `?T` | `T` or `none` | `or { default }`, `if v := e`, `?` |
| `!T` | `T` or `IError` | `or { return error(...) }`, `if v := e`, `!` |

- `?T` carries no error. Do not convert an absence into an error unless absence is a
  failure at that layer.
- In an `or` block on a result, `err` is bound to the `IError`. On an option, no `err`
  is bound.
- `x!` propagates; in `main` it panics.
- `x or { 0 }` discards the failure. Only use a default that is correct on failure.
- `if v := e {}` unwraps both options and results; the else branch sees `err` for a
  result.
- A function may return `!` or `?` with no payload.
- Only one option/result in a multi-return; wrap the rest in a struct.
- Custom errors implement `IError` (`msg() string`, `code() int`); embed `Error` for
  defaults.

## Mutability

- Variables, struct fields, and arguments are immutable by default.
- `mut` is required at declaration and at the call: `f(mut x)`.
- `fn (mut r T)` mutates the caller's value and requires the caller to pass a `mut`
  variable. `fn (r &T)` is read-only; `fn (mut r &T)` is rejected by the compiler.
- `mut` on a primitive argument is not allowed.
- Shadowing is a compile error.
- `shared` adds a hidden mutex; `lock` writes, `rlock` reads.

## Sum types and match

- `type Expr = IntLit(int) | Void` is a named-variant sum; `type World = Moon | Mars`
  is a type-variant sum.
- `match` must cover every variant or have `else`.
- `if x is Mars {}` smart-casts; `x as Mars` panics if the variant is wrong.
- A mutable sum needs `if mut x is Mars` / `match mut x`.
- Reference variants (`&T`) are distinct from value variants and must be matched as
  declared.
- Recursive payloads need indirection: `child &Tree`, `children []Tree`, or `?&Tree`.

## Enums

- Values start at 0 and increment; explicit integer values are allowed.
- `@[flag]` gives `|`, `has`, `all`, `set`, `clear`, `toggle`, `is_empty`.
- `Enum.from(value)` returns `!Enum`; `.str()` gives the variant name.
- Exhaustive `match` is the point: a new variant breaks every incomplete branch.

## Strings, arrays, maps

- Strings are immutable UTF-8. `.len` and indexing are **bytes**; `.runes()` is
  characters. `s[1..3]` slices bytes.
- Single quotes are canonical; `v fmt` rewrites double quotes where it can.
- Use interpolation, not `+`, for mixed types. `+` requires two strings.
- Dynamic arrays: `[]T`, `<<`, `in`, `.filter`, `.map`, `.any`, `.all`, `.sort`.
  `[]T{cap: n}` avoids reallocations.
- A slice is a view over the parent's storage. With a mutable parent it aliases: writes
  through either are visible through the other, until the parent reallocates and the
  slice detaches. `mut s := immutable[1..]` clones, because the immutable parent must not
  change (the compiler prints a notice). `.clone()` copies explicitly; `unsafe { a[..] }`
  shares memory.
- Fixed arrays `[N]T` are stack values; `a[..]` converts to a dynamic array.
- Maps preserve insertion order. A missing key returns the zero value; use `in`,
  `if v := m[key]`, or `or {}`. `@[strict_map_index]` requires a guard.
- Map keys may be strings, runes, integers, floats, voidptr, enums, or fixed arrays of
  those; structs are not allowed.

## Comptime

- `$if linux {}`, `$if windows {}`, `$if debug {}`, `$if custom ? {}`,
  `$for field in T.fields {}`.
- The compiler **excludes** the non-matching branch entirely; this is why a
  platform-specific import inside `$if` is safe.
- `$embed_file`, `$tmpl`, `$env`, `typeof`, `T.name`, `T.fields`, `T.values`,
  `@FN`, `@FILE`, `@LINE`, `@VEXE`, `@VMODROOT`.
- `$if` is not a runtime `if`. A runtime branch that names a platform-only symbol still
  has to compile on every platform.
- `-d name` defines a flag. Files named `*_d_name.v` are included only with that flag.

## Conversions

- No implicit narrowing or sign-changing conversion. `int` is platform-width; use
  `i32`/`i64` when the width matters.
- Literals adapt to the destination type; typed values do not.
- Use `strconv` for parsing with errors, `.int()` only when the input is known valid.
- `-check-overflow` and `-check-casts` turn silent wrap/truncation into runtime panics.
  They do not analyse at compile time.

## Control flow

- `for x in xs`, `for i, x in xs`, `for mut x in xs`, `for i in 0 .. n`,
  `for cond {}`, `for {}`, C-style `for i := 0; i < n; i++`.
- Labels: `outer: for ... { continue outer }`.
- `match` ranges use `...` (inclusive); `..` is exclusive and invalid in a match
  branch.
- `defer {}` runs at scope exit; `defer(fn) {}` at function exit; `$res()` reads the
  return value.
- `recover()` in a deferred block stops a panic and returns its message; signals and
  segmentation faults are not recoverable.
