# The V toolchain

Source: `cmd/v`, `cmd/tools`, `vlib/v/help`, and the V documentation. Commands are
stable in 0.5.x; check `v help <command>` on the installed compiler.

## Flags go before the command

```bash
v -prod run main.v       # correct
v run main.v -prod       # -prod is passed to the program
v run main.v -- --flag   # program arguments after --
```

`VFLAGS` supplies options to every invocation: `export VFLAGS='-cc gcc'`.

## The loop

| Question | Command |
|---|---|
| Does it type-check? | `v -check file.v` (`-shared` for a library) |
| Is it formatted? | `v fmt -verify file.v` |
| Format it | `v fmt -w file.v` |
| Any suspicious constructs? | `v vet -W file.v` |
| Does it build and run? | `v run .` |
| Do the tests pass? | `v test .` |
| Release build | `v -prod -cstrict .` |
| What does this symbol resolve to? | `v where fn name` |
| Which import pulls a module in? | `v mod why module.name` |
| Show the AST | `v ast -p file.v` |
| Shrink a compiler error | `v reduce -m 'error text' file.v` |
| Environment diagnostics | `v doctor` |

## Formatter and vetter

- `v fmt -w` rewrites; `v fmt -verify` exits 1 if any file would change.
- `// vfmt off` / `// vfmt on` protect a block; use sparingly.
- `v vet -W` promotes warnings to errors. It checks public function docs, space
  indentation, trailing whitespace, and `x in [single]`.
- `v vet -F` reports long and empty functions; `-r` repeated expressions; `-I` possible
  inlining. These are notices, not failures.
- `v test-fmt` verifies every `.v` file in a V compiler checkout; `v -silent test-fmt` is
  the CI form. For your own project, use `v fmt -verify`.

## Build flags worth knowing

| Flag | Effect |
|---|---|
| `-prod` | optimise; all warnings become errors; asserts removed |
| `-g` | add V line numbers to the binary |
| `-cg` | add C line numbers too |
| `-cc gcc\|clang\|tcc\|msvc` | pick the C compiler |
| `-cflags` / `-ldflags` | flags for compile / link |
| `-cstrict` | strict C diagnostics |
| `-W` | warnings as errors |
| `-o out` | output name; `-o -` writes C to stdout |
| `-shared` | shared library; also required for `-check` on a library |
| `-gc none` / `-prealloc` | memory mode |
| `-autofree` | compiler-inserted frees (WIP) |
| `-ownership` | static move checking |
| `-race` | ThreadSanitizer |
| `-check-overflow` / `-check-casts` | runtime panics on wrap/truncation |
| `-no-skip-unused` | keep unreachable functions (needed for coverage) |
| `-stats` / `-show-timings` | test timings / compile phases |
| `-prof file.txt` | per-function call counts and time (not thread-safe) |
| `-showcc` / `-keepc` | print / keep the generated C |

## Tests

- `v test .` runs every `_test.v` below the path. `v file_test.v` runs one file.
- `VTEST_ONLY='substr'` filters by path; `VTEST_ONLY_FN='test_name'` filters by function.
  Both take comma-separated lists; function filters are glob patterns.
- `v -silent test .` hides passing tests; `v -stats test .` adds timings.
- Coverage: `v -no-skip-unused -coverage vcov/ test .`, then `v cover vcov/`,
  `v cover vcov/ --uncovered`, `v cover vcov/ --functions`,
  `v cover vcov/ --json coverage.json`.
- `testdata/` beside the tests is ignored by the runner.
- `testsuite_begin` / `testsuite_end` are per-file hooks.
- `@[assert_continues]` or `-assert continues` prints failures without aborting.

## v.mod

A project has one `v.mod` at its root:

```v ignore
Module {
	name: 'myapp'
	description: 'What it does.'
	version: '0.1.0'
	license: 'MIT'
	repo_url: 'https://github.com/me/myapp'
	dependencies: [
		'vlang.markdown'
	]
}
```

Only `name` and `dependencies` affect the compiler. Import resolution, in order:

1. the importing file's directory (so `import foo` finds `foo/` beside it)
2. `-path` roots, when supplied
3. `vlib/` of the compiler
4. `~/.vmodules` (or `$VMODULES`)
5. parent directories, which finds a sibling project

A project install (`v install` with no package, or `--local`) writes `v.mod.lock` next to
`v.mod`. Commit it: without a lock entry, `v install` moves a dependency to its latest
revision; with one, it installs the locked revision. `v install --locked` fails instead of
resolving anything anew, and `--frozen` also leaves the lock untouched. Dependency strings
can be semver ranges (`lib@^1.2.0`), exact tags, branches, or URLs.

## Project creation

```bash
v new myapp           # executable in myapp/
v new --lib mylib     # library
v new --web myapp     # veb app
v init                # project in the current directory
```

Flags for `new` go **before** the project name: `v new --lib mylib`, not
`v new mylib --lib`. `v new` creates `v.mod`, `.editorconfig`, `.gitattributes`, and
`.gitignore`, and runs `git init` when git is available. Add `--agents-md` to also
generate an `AGENTS.md`.

## Scripts

A `.vsh` file gets an implicit `import os` and unqualified `os` calls. `v script.vsh`
caches the built executable; `v run script.vsh` rebuilds every time. Use a script for
tool orchestration, and a normal project with tests for domain logic.

## MCP

`v mcp serve` exposes the parser, checker, formatter, and stdlib docs over MCP. Use it
when an agent needs `v_check`, `v_symbols`, `v_references`, `v_stdlib_doc`, or a guarded
rename instead of shelling out and re-reading files.
