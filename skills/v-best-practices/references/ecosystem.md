# Modules, projects, and the ecosystem

Source: `doc/docs.md` (Modules, Package management, Writing documentation),
`cmd/tools/vpm`, and the `vlib` layout.

## Layout

```text
myapp/
├── v.mod
├── main.v              module main
└── myapp/
    ├── config.v        module myapp
    └── config_test.v   module myapp
```

- Every `.v` file in a directory belongs to one module; the directory name is the module
  name, not the full import path.
- The top-level project directory (the one compiled with `v .`) may hold files with
  different `module` declarations. That is for prototyping only; organise real code into
  directory modules.
- There is no `src/` or `modules/` magic. The directory holding a module sits at the
  project root (beside `v.mod`) or under it; imports are the path under that root.
- A library's test file stays beside the source and declares the same module for
  internal tests, or imports it for external tests.

## Visibility

- Top-level declarations are private unless marked `pub`.
- Struct fields are private and immutable unless `pub:`, `mut:`, or `pub mut:`.
- A public function may return a private type; callers can read its public fields but
  cannot name the type.
- `__global:` is almost always wrong. Do not use it to emulate a setter.
- `fn init()` runs once when a module is imported; `fn cleanup()` runs at program exit,
  in reverse init order. Both are private by definition.

## Dependencies

```bash
v install                    # resolve v.mod and write v.mod.lock
v install 'vsl@^0.1.47'      # semver range, recorded in v.mod
v install --locked           # fail if v.mod.lock is stale (CI)
v install --frozen           # use the lock without rewriting it
v update -p vsl --precise v0.1.45
v outdated                   # Current, Upgradable, Resolvable, Latest
v list                       # what is installed
v search keyword             # search vpm.vlang.io
v why vsl                    # requirement chain with constraints
v mod why vsl                # import chain that pulls it in
v link                       # symlink this project into ~/.vmodules
v unlink                     # remove that symlink
```

- Add the dependency to `v.mod` as well as installing it; a project install writes
  `v.mod.lock` next to it. Commit the lockfile so everyone builds the same revisions.
- `v install --locked` fails instead of resolving a new revision; use it in CI.
  `--frozen` also refuses to rewrite the lock.
- Dependency strings may be bare names, exact tags, branches, URLs, or semver ranges
  (`lib@^1.2.0`, `lib@>=0.4.0 <0.6.0`). A bare name follows the default branch.
- Root `dev_dependencies` are resolved and locked with the regular dependencies but are
  not propagated as requirements of the modules that import this one.
- `v link` is for developing two of your own projects together. Never publish with a
  link in place.
- A module can be installed from a Git URL; `v install` delegates to Git.

## Packages worth knowing

| Import | Use |
|---|---|
| `os` | files, directories, processes, environment |
| `strings` | builder, split, replace, trim |
| `strconv` | fallible numeric parsing and formatting |
| `arrays` | sort, chunk, window, parallel map/run |
| `maps` | map helpers |
| `json2` | encode/decode with struct tags |
| `time` | time, duration, formatting |
| `net.http` | HTTP client |
| `veb` | web framework |
| `sync` | Mutex, RwMutex, WaitGroup, Once; `sync.pool` for worker pools |
| `sync.stdatomic` | atomics |
| `benchmark` | micro-benchmarks |
| `log` | structured logging |
| `flag` | command-line flags |
| `term` | colour and terminal control |

Check `v doc <module>` or `v where fn <name>` before assuming an API. `vlib` is the
source of truth and is readable.

## Documentation

- A public declaration gets a comment directly above it, starting with its name:
  `// parse_header parses one header line.`
- Present tense, one idea per sentence, examples where the contract is subtle.
- Document the failure modes of a `!T` function: what fails, what the error means, and
  whether the caller can retry.
- The module overview goes in the first comment after `module`.
- `v doc .` renders a directory's docs; `v missdoc .` lists missing public docs.
- `v check-md` validates Markdown code fences against examples; useful for a docs-heavy
  repo.

## CI

A minimal V CI job:

```yaml
- uses: actions/checkout@v7
- uses: vlang/setup-v@v1
  with:
    version: '0.5.2'
- run: v fmt -verify .
- run: v vet -W .
- run: v -check .
- run: v test .
- run: v -prod -cstrict .
```

For a library, use `v -check -shared .` and `v -prod -shared -cstrict .`. Add
`v -race test .` on Linux/macOS with clang when the code is concurrent.

## Scripts and tools

- `.vsh` for build/release/maintenance glue.
- `v new` / `v init` for a project skeleton.
- `v where`, `v ast`, `v reduce`, `v mod why`, `v doctor`, `v env` for diagnosis.
- `v mcp serve` when an agent should ask the compiler rather than guess.

## Licensing and attribution

V itself is MIT. When you copy code from `vlib`, keep the copyright header. When you
port code from another project, its licence still applies; record the origin in
`PORTING.md`.
