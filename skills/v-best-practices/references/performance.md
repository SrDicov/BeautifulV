# Performance, in the order that pays

Source: `doc/docs.md` (Performance tuning, Atomics, noalloc), `vlib/benchmark`, and the
compiler's timing and allocation tests. Profile first; almost every item here is a
last resort.

## 1. Build mode

```bash
v -prod run .              # optimised C, asserts removed
v -prof prof.txt run .     # per-function call counts and time (not thread-safe)
v -show-timings .          # compiler phase timings
v -cc clang -prod run .    # clang often produces better code than tcc
```

`-prod` treats every compiler warning as an error, so an unused variable or an expired deprecation fails the build. `-W` gives the same warning-as-error behaviour to a debug build. A debug build is not a benchmark.

## 2. Data layout and allocation

- Values, not pointers, unless identity or mutation requires a pointer.
- Fixed arrays (`[N]T`) are stack storage: the right buffer for parsers, codecs, and
  small hot loops. A dynamic array allocates and bounds-checks.
- `[]T{cap: n}` reserves capacity and avoids repeated growth. `len` is initialised
  elements; `cap` is reserved space.
- `strings.Builder` (`strings.new_builder(n)`, `.write_string`, `.str()`) for repeated
  concatenation. Repeated `s += x` reallocates.
- A slice normally aliases its parent and allocates nothing. The hidden allocation is
  `mut s := immutable[1..]`, which clones to protect the immutable parent. In a hot loop,
  avoid an accidental `.clone()` and an unnecessary `mut` slice.
- Maps are O(1) but allocate. A switch over an enum is a jump table and no allocation.
- `@[minify]` reorders struct fields to reduce padding. It breaks binary layout and
  reflection assumptions; use only for in-memory structs.

## 3. Compute

- `@[inline]` on small, frequently called functions. Large functions bloat the
  instruction cache; measure.
- `@[direct_array_access]` removes bounds checks in a function. Only use it when the
  index range is proved by the surrounding code (a loop bounded by `.len`, an explicit
  check).
- `@[packed]` reduces struct size at the cost of unaligned access; not portable to
  architectures that reject unaligned loads.
- `@[aligned]` raises alignment; use it for SIMD or a documented hardware requirement.
- `_likely_(cond)` / `_unlikely_(cond)` are branch hints. A wrong hint costs more than
  no hint.
- `-fast-math` allows the C compiler to break IEEE semantics. Never in finance,
  scientific computing, or anything that compares floats for equality.
- `-cflags -march=native` targets the build machine only. Do not ship it.
- PGO: build with `-cflags -fprofile-generate`, run a representative workload, merge
  with `llvm-profdata`, rebuild with `-cflags -fprofile-use=...`. Worth it only for a
  stable, performance-critical binary.
- `-compress` runs `upx`; it trades startup time and antivirus false positives for size.

## 4. Concurrency

- `parallel.amap` preserves order and returns results; `parallel.run` discards them.
  Both default to `runtime.nr_jobs()` workers (`VJOBS`).
- `spawn` is an OS thread; do not spawn one per item in a million-item loop.
- `shared` maps lock each single operation; batch a read-modify-write under one `lock`.
- Atomics (`sync.stdatomic`) fit one counter or flag; two values that must agree need a
  mutex.

## 5. Memory mode

- Default GC is usually fast enough. `-gc none` removes tracing pauses but makes you
  responsible for every allocation and every cycle.
- `-prealloc` can speed up a compiler-like one-shot workload; it is wrong for a server.
- Measure before changing the mode: `-prof` for runtime cost, `-show-timings` for compile
  time, and the allocator's own counters for allocation volume.

## 6. Compile speed

V's compile speed is part of the product. Keep it:

- Prefer small modules and few generic instantiations.
- `@[if flag ?]` and `$if` exclude code from the build, but V still type-checks it.
  unreachable functions are dropped by default; `-no-skip-unused` disables that (and is
  required for coverage).
- `-no-skip-unused` is for coverage, not production.
