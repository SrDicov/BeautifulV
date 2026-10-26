# AGENTS.md

Content repository, not a V application: three Agent Skills (`skills/`), a formatting and
vetting gate (`lints/`), and Python validators. The only V files are intentional fixtures:
`lints/check/` (one good file, one deliberately bad), the eval inputs, and the example
review's target. There is no root `v.mod` and nothing to build at the repository root.

## Verify before committing

```bash
python3 scripts/check-skill.py              # CI; --json for machine output; optional root arg
python3 -m unittest discover -s tests -v    # checker tests; CI runs them
bash scripts/check-lints.sh                 # CI; needs `v` on PATH
```

Single test: `python3 tests/test_check_skill.py CheckSkillTests.test_real_repo_passes`.
When adding a rule to `scripts/check-skill.py`, add a fixture case to `tests/`.

The gate needs V 0.5.2. CI installs the commit `5aa8542` with `vlang/setup-v@v1`; locally,
build the V checkout or put the compiler on `PATH`. When bumping the pinned commit,
recompile every `v` example block and rerun `bash scripts/check-lints.sh`.

## What the checker enforces

- `SKILL.md` `name` must equal its folder name (lowercase letters, digits, hyphens).
  Allowed frontmatter keys only: `name`, `description`, `license`, `compatibility`,
  `metadata`, `allowed-tools`.
- `description` ≤ 1024 chars; `SKILL.md` body > 500 lines is a warning.
- Every relative link, image, and `references/...` mention must resolve. A file under
  `skills/*/references/` not linked from `SKILL.md` is an orphan error.
- Evals: case `name` equals its folder; `case.yaml` needs `schema_version`, `name`,
  `expected_outcome`; `prompt.md` frontmatter needs `tags`, `max_turns`,
  `timeout_seconds`; graders are contiguous `NN-slug.md` files with `type`, `focus`,
  `weight` frontmatter and a non-empty body; `place-files.sh` references must exist
  under `files/`.
- Version lives in `.claude-plugin/plugin.json` and must match the names in
  `.claude-plugin/marketplace.json`. `CHANGELOG.md` needs `[Unreleased]` and/or
  `[<version>]`, plus compare links containing the version. On a version bump, also
  repoint the `[Unreleased]` compare URL at the new tag.
- README's `Layout` tree is user-facing; update it when adding or removing files.

## The gate

- `lints/gate.sh` runs `v fmt -verify` then `v vet -W`. It must report all diagnostics: do
  not add `set -e`, or a formatting failure hides the vet failures.
- `lints/check/good.v` must pass; `lints/check/bad.v` must keep firing all five expected
  diagnostics. `scripts/check-lints.sh` asserts the exact strings, so changing either
  fixture means updating the expected list.
- `bad.v` intentionally contains space indentation and trailing whitespace. Do not run
  `v fmt -w` on it.
- Do not claim the gate catches API design, errors, memory, or concurrency. It cannot;
  `lints/README.md` has the honest split.

## Eval conventions

- Eval inputs are deliberately bad: `skills/v-review/evals/*/files/inventory.v` and the
  refactor copy must keep their panics, swallowed errors, and undocumented public API.
  The graders depend on those exact defects.
- `skills/v-port/evals/port-c-config-parser/files/` is real C and must compile and pass:
  `cc -std=c99 -Wall -Wextra -Werror config.c test_config.c -o /tmp/test_config`.
- `skills/v-port/evals/port-python-task-graph/files/` is real Python and must pass:
  `python3 -m unittest test_taskgraph.py` from that directory.
- Evals are billed model runs. From a skill directory:
  `claude plugin eval . --allow-tools Write,Edit,Bash --scaffold`. Do not run unprompted.

## Content conventions

- Every rule needs a published source: `doc/docs.md`, the compiler/tooling source, `vlib`,
  or an accepted V issue. See `CONTRIBUTING.md`.
- V examples in `skills/` and the README must compile with V 0.5.2. Mark a fragment that
  cannot stand alone `v ignore`.
- Use V vocabulary precisely: `?T`/`!T`, `mut` receiver, `spawn` handle, `shared`/`lock`,
  `v -check`, `v vet -W`. Do not describe V in Rust or Go terms.
- Keep `skills/v-best-practices/SKILL.md` near 500 lines; deeper material belongs in
  `references/` and must stay linked.
- No Claude-specific instructions in skills; they are standard Agent Skills.
- `assets/banner.svg` is hand-authored documentation, not product code.
