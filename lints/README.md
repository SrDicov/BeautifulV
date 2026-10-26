# The V style gate

V has no clippy-style configuration file and no custom-lint plugin API. What it does
have is a fast formatter and a built-in vetter, both driven by the compiler's own
parser. `gate.sh` runs the two commands that fail a build on the rules V can check
mechanically:

```bash
bash lints/gate.sh .          # a file, a directory, or several
```

Copy `gate.sh` into a project, or paste the two commands into CI:

```bash
v fmt -verify .
v vet -W .
```

`v fmt -verify` exits non-zero when a file would be reformatted. `v vet -W` promotes
its warnings to errors, so a missing public-function doc comment fails the build.
`-W` is the important half: without it, vet prints warnings and exits zero.

## What the gate catches

| Rule | Command | Diagnostic |
|---|---|---|
| Tabs for indentation, no spaces | `v fmt -verify`, `v vet -W` | `is not vfmt'ed`, `using spaces for indentation` |
| No trailing whitespace | `v vet -W` | `Looks like you have trailing whitespace` |
| Public functions are documented | `v vet -W` | `Function documentation seems to be missing` |
| A doc comment starts with the function name | `v vet -W` | `A function name is missing from the documentation` |
| `x == 1`, not `x in [1]` | `v vet -W` | `Use ... instead of ... in [...]` |
| One-line comments stay attached to the right line | `v fmt -verify` | `is not vfmt'ed` |

## Optional vet passes

These are notices, not errors, and they are noisy on small examples. Run them when
reviewing, not on every commit:

```bash
v vet -F .        # functions over 300 lines, and empty functions
v vet -r .        # expressions repeated at least 10 times in one function
v vet -I .        # short functions called often enough to inline
```

The thresholds are environment variables such as `VET_LONG_FNS_CUTOFF`,
`VET_CALLEXPR_CUTOFF`, and `VET_FNS_CALL_CUTOFF`.

## What the gate cannot catch

The gate is formatting and documentation. It does not check API design, error
handling, memory mode, or concurrency. Those are in the skills and are enforced by
review, tests, and the compiler:

| Rule | Enforced by |
|---|---|
| Types check | `v -check .` (add `-shared` for a library) |
| Tests pass | `v test .` |
| No warnings in release | `v -prod .` (all warnings fail; asserts are removed) |
| Stricter C diagnostics | `v -cstrict .` |
| Integer overflow and lossy casts | `v -check-overflow -check-casts .` (runtime panics) |
| Use after move | `v -ownership .` (opt-in) |
| Data races | `v -race run .`, `v -race test .` |
| No allocations in a function | `@[noalloc]` on the function |
| No `unsafe` without a reason | review |

## The self-check

`lints/check/` holds one good file and one deliberately bad file. From the repository
root:

```bash
bash scripts/check-lints.sh
```

It fails if the good fixture stops passing, if the bad fixture starts passing, or if
any diagnostic above disappears. That is the equivalent of proving a lint table: the
gate is tested against the compiler version in CI, not assumed.
