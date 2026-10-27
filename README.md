<p align="center">
  <img src="assets/banner.svg" alt="A dark banner with a large V monogram and the subtitle Idiomatic V, distilled.">
</p>

# Beautiful V

Three standard [Agent Skills](https://agentskills.io) and a style gate that teach coding
agents to write, review, and port idiomatic V:

- **`v-best-practices`** loads whenever an agent writes or changes V, and steers it toward
  code that is fast, light, and safe: values before allocation, options and results
  before sentinels, private fields before mutable state, and `unsafe` only at a boundary.
- **`v-review`** reviews existing V code, a module, or a diff, and writes a prioritized
  report where every finding cites a file and line, the rule it breaks, and a concrete fix.
- **`v-port`** ports C, C++, Python, Go, JavaScript, TypeScript, or Rust to idiomatic V
  with the same behaviour, porting the original tests first so the two versions can be
  proved equivalent.
- **`lints/`** is a drop-in `v fmt -verify` + `v vet -W` gate, with a fixture that proves
  every diagnostic still fires.

They work with Claude Code, OpenAI Codex, GitHub Copilot, Cursor, Gemini CLI, and any other
agent that reads the Agent Skills format. Every V example in the skills and this README was checked against V 0.5.2,
and the gate is tested against the compiler in CI.

V itself ships a set of agent skills (`v skills list`) that cover the workflow, memory,
concurrency, veb, and the MCP server. Beautiful V is complementary: it is the writing,
review, and porting layer, plus a gate you can drop into any project.

The guidance is distilled from the V language reference, the compiler and tooling source,
and the standard library, with condensed digests in the skill so an agent can consult the
reasoning without leaving the task:

- [`doc/docs.md`](https://github.com/vlang/v/blob/master/doc/docs.md) — the language
  reference; the compiler is the specification when they disagree.
- [`cmd/v`](https://github.com/vlang/v/tree/master/cmd/v) and
  [`cmd/tools`](https://github.com/vlang/v/tree/master/cmd/tools) — flags, `v fmt`, `v vet`,
  `v test`, `v check-md`, `v doc`.
- [`vlib`](https://github.com/vlang/v/tree/master/vlib) — the standard library and the
  idiomatic source to imitate.
- [`doc/ownership.md`](https://github.com/vlang/v/blob/master/doc/ownership.md) — the
  opt-in move checker.
- The [V documentation site](https://docs.vlang.io/) and
  [`modules.vlang.io`](https://modules.vlang.io/) — the split reference and stdlib docs.

## What it changes

Asked for a small SemVer parser, an agent without the skill tends to expose mutable
fields, parse with `.int()`, and return a zero value on failure. With the skill, it keeps
the fields private, returns a typed error, and documents the failure modes.

```v
// Without the skill
pub struct Version {
pub mut:
	major int
	minor int
	patch int
}

pub fn parse(s string) Version {
	parts := s.split('.')
	return Version{parts[0].int(), parts[1].int(), parts[2].int()}
}
```

```v
// With the skill
import strconv

pub struct Version {
	major int
	minor int
	patch int
}

// parse reads a MAJOR.MINOR.PATCH version.
pub fn parse(s string) !Version {
	parts := s.split('.')
	if parts.len != 3 {
		return error('expected 3 parts, got ${parts.len}')
	}
	major := strconv.atoi(parts[0]) or { return error('bad major ${parts[0]}') }
	minor := strconv.atoi(parts[1]) or { return error('bad minor ${parts[1]}') }
	patch := strconv.atoi(parts[2]) or { return error('bad patch ${parts[2]}') }
	return Version{
		major: major
		minor: minor
		patch: patch
	}
}
```

The second version cannot panic on bad input, cannot be built in an inconsistent state
from another module, and tells the caller which part failed.

## Install

Each skill is a folder under `skills/`: `v-best-practices/`, `v-review/`, and `v-port/`.
Install all three; the review and port skills lean on the writing skill for style.

### Any agent, one command

The [`skills`](https://github.com/vercel-labs/skills) installer supports most agents
(Node 22.20 or later):

```bash
npx skills add SrDicov/BeautifulV
```

Add `-g` to install for your user instead of the current project, or `-a <agent>` to pick
an agent, such as `-a codex` or `-a cursor`.

### Claude Code

Install it as a plugin so updates arrive with one command:

```bash
claude plugin install beautiful-v --marketplace SrDicov/BeautifulV
```

Inside a session, the same thing is:

```
/plugin marketplace add SrDicov/BeautifulV
/plugin install beautiful-v@beautiful-v
```

This installs all three skills. They appear as `/beautiful-v:v-best-practices`,
`/beautiful-v:v-review`, and `/beautiful-v:v-port`, though you rarely need to call them,
since they trigger on their own. To try it for one session without installing, clone the
repo and run `claude --plugin-dir ./BeautifulV`.

### GitHub Copilot

With GitHub CLI 2.90 or later:

```bash
gh skill install SrDicov/BeautifulV v-best-practices
gh skill install SrDicov/BeautifulV v-review
gh skill install SrDicov/BeautifulV v-port
```

This installs into the current project by default. Add `--scope user` to install for your
user, or `--agent <name>` to install for a different agent.

### Gemini CLI

```bash
gemini skills install https://github.com/SrDicov/BeautifulV --path skills/v-best-practices
gemini skills install https://github.com/SrDicov/BeautifulV --path skills/v-review
gemini skills install https://github.com/SrDicov/BeautifulV --path skills/v-port
```

### OpenCode

OpenCode discovers the skills from `~/.config/opencode/skills/`, but it does not turn
them into slash commands by itself. The matching command files add `/v-best-practices`,
`/v-review`, and `/v-port`:

```bash
git clone --depth 1 https://github.com/SrDicov/BeautifulV /tmp/BeautifulV
mkdir -p ~/.config/opencode/commands ~/.config/opencode/skills
cp -R /tmp/BeautifulV/skills/v-* ~/.config/opencode/skills/
cp /tmp/BeautifulV/opencode/commands/*.md ~/.config/opencode/commands/
opencode reload
```

Restart OpenCode if the commands do not appear in the `/` menu.

### OpenAI Codex, Cursor, or anything else: copy the folder

Clone the repo and copy the three skill folders into your agent's skills directory. Most
agents now read the shared `.agents/skills/` location:

```bash
git clone https://github.com/SrDicov/BeautifulV
mkdir -p ~/.agents/skills
cp -R BeautifulV/skills/v-* ~/.agents/skills/
```

| Agent | User directory | Project directory |
|---|---|---|
| Claude Code | `~/.claude/skills/` | `.claude/skills/` |
| OpenAI Codex | `~/.agents/skills/` | `.agents/skills/` |
| GitHub Copilot | `~/.copilot/skills/` or `~/.agents/skills/` | `.github/skills/` or `.agents/skills/` |
| Cursor | `~/.cursor/skills/` or `~/.agents/skills/` | `.cursor/skills/` or `.agents/skills/` |
| Gemini CLI | `~/.gemini/skills/` or `~/.agents/skills/` | `.gemini/skills/` or `.agents/skills/` |

Claude Code does not read `.agents/skills/`, so use its own directory or the plugin.
Restart the agent if the skill does not show up right away.

If the V compiler is installed, `v skills add` manages its own bundled skills; use it for
the workflow and memory material, and this repository for the writing, review, and porting
skills.

### claude.ai

claude.ai takes one zip per skill, with the skill folder as the zip root:

```bash
cd skills
zip -r ../v-best-practices.zip v-best-practices -x '*.DS_Store' -x '*/evals/*'
zip -r ../v-review.zip v-review -x '*.DS_Store' -x '*/evals/*'
zip -r ../v-port.zip v-port -x '*.DS_Store' -x '*/evals/*'
```

Upload each under **Customize > Skills**. Code execution must be enabled first.

## Reviewing existing code

Ask your agent to review a file, module, or diff, for example "review src/parser.v" or
"what would you change in this PR". The `v-review` skill runs `v fmt -verify`, `v -check`,
`v vet -W`, and the tests when a toolchain is available, then writes a report with three
tiers: must fix, should fix, and consider. Every finding gives a `file:line` location, the
rule it breaks with its source, and the smallest code change that fixes it. Each report
ends with what the code already does well.

[`examples/review-inventory.md`](examples/review-inventory.md) is a real review of the
deliberately messy module in the eval cases.

## Porting code to V

Ask your agent to port a module, for example "port parser.c to V" or "rewrite this Python
service in V". The `v-port` skill reads the original and its tests, ports the tests first,
designs a V API instead of transliterating, and verifies with `v test` and `v vet -W`. It
maps each language's habits to V:

- `NULL`, `None`, `nil`, and `undefined` become `?T`.
- error codes, exceptions, and `error` become `!T` with a typed error.
- out-parameters become return values or a small result struct.
- flag ints and string modes become enums; boolean mode parameters become enums.
- dictionaries become V maps, which preserve insertion order.
- classes and inheritance become structs, embedding, and interfaces.
- `with`, `finally`, and RAII become `defer`.
- goroutines, threads, and `async` become `spawn`, `parallel.amap`, channels, and `shared`.

Each port comes with a `PORTING.md` that maps the old API and tests to the new ones and
lists every deliberate difference.

## Enforcing the rules

V has no clippy-style configuration file. The formatter and the built-in vetter are the
mechanical gate:

```bash
v fmt -verify .
v vet -W .
```

Copy [`lints/gate.sh`](lints/gate.sh) into your project, or run those two lines in CI.
`v fmt -verify` fails when a file would be reformatted. `v vet -W` fails on a missing
public-function doc comment, space indentation, trailing whitespace, and the
single-element `in` check.

[`lints/check/`](lints/check/good.v) holds one good file and one deliberately bad file.
From the repository root:

```bash
bash scripts/check-lints.sh
```

It fails if the good fixture stops passing, if the bad fixture starts passing, or if any
expected diagnostic disappears. [`lints/README.md`](lints/README.md) lists what the gate
catches and, just as importantly, what it cannot: API design, error handling, memory mode,
and concurrency are enforced by review, tests, and the compiler.

## What the writing skill covers

[`skills/v-best-practices/SKILL.md`](skills/v-best-practices/SKILL.md) is under 500 lines,
organized so an agent can scan it while coding:

1. Mindset: V-shaped code, simplicity, invalid states unrepresentable
2. Pre-handback checklist: fmt, check, vet, test, release build
3. Variables, mutability, receivers, closures, defer
4. Types and API design: naming, structs, enums, sum types, interfaces, generics
5. Errors: `?T` vs `!T`, `or` blocks, custom errors, panic policy
6. Idioms: strings, arrays, fixed arrays, maps, match, defer
7. Memory and safety: GC modes, ownership, `unsafe`, C interop
8. Concurrency: `parallel`, `spawn`, channels, `shared`, atomics, race detector
9. Performance, in order: `-prod`, allocation, layout, then compiler attributes
10. Modules, `v.mod`, dependencies, scripts
11. Documentation and tests
12. Final checklist

[`skills/v-best-practices/references/`](skills/v-best-practices/references/language.md)
holds one digest per area: language rules, memory, performance, toolchain, and ecosystem.

## Layout

```
BeautifulV/
├── .claude-plugin/
│   ├── plugin.json         plugin manifest
│   └── marketplace.json    lets this repo act as its own marketplace
├── skills/
│   ├── v-best-practices/   writing skill
│   │   ├── SKILL.md
│   │   ├── references/     one digest per area
│   │   └── evals/
│   ├── v-review/           review skill
│   │   ├── SKILL.md
│   │   └── evals/
│   └── v-port/             porting skill
│       ├── SKILL.md
│       └── evals/          a C module and a Python module to port
├── opencode/
│   └── commands/           slash commands that load each skill
├── lints/
│   ├── gate.sh             drop-in fmt + vet gate
│   ├── README.md           what the gate catches and what it cannot
│   └── check/              one good and one deliberately bad V file
├── scripts/
│   ├── check-skill.py      validates skills, evals, and manifests
│   └── check-lints.sh      proves the gate fires
├── examples/
│   └── review-inventory.md a real review report
├── CHANGELOG.md
├── CONTRIBUTING.md
└── README.md
```

## Testing the skills

`skills/v-best-practices/evals/` holds three cases in the layout `claude plugin eval`
reads: a library API design task, a CLI task, and a refactor-this-module task with an
input file. Each case has the prompt, a description of a good result, and one assertion per
file under `graders/`. Run them with and without the skill from the skill directory:

```bash
claude plugin eval . --allow-tools Write,Edit,Bash --scaffold
```

`skills/v-review/evals/` holds a review case for the same messy module, and
`skills/v-port/evals/` holds a C module and a Python module to port, each with its original
test suite. Run them the same way from each skill's directory.

These are real model runs and are billed. The gate is cheaper: `bash scripts/check-lints.sh`
runs the compiler against the fixtures on every change.

## Updating

Edit the skill, bump `version` in `.claude-plugin/plugin.json`, commit, and push. Claude
Code plugin users pick up the new version with:

```bash
claude plugin update beautiful-v
```

Anyone who copied the folder or used an installer re-runs the install command. Record each
release in [CHANGELOG.md](CHANGELOG.md).

Validate before pushing:

```bash
python3 scripts/check-skill.py
claude plugin validate .
```

`check-skill.py` checks skill frontmatter, links, eval cases, and the plugin manifests.
Pass `--json` for machine-readable output. The Python unit tests for the checker are in
[`tests/`](tests/test_check_skill.py).

## Contributing

Issues and pull requests are welcome. Every rule needs a published source; see
[CONTRIBUTING.md](CONTRIBUTING.md) for how to propose a rule, report a review false
positive, or add an eval case.

## Sources and attribution

This project is a V-focused fork of
[BeautifulRust](https://github.com/morganlinton/BeautifulRust), which established the
structure: three skills, eval cases, a style checker, and a drop-in lint configuration.
The V content is original and is distilled from the sources listed above. The digests are
paraphrased summaries written for this project; read the originals for the full reasoning.

| Source | Author | License |
|---|---|---|
| [V documentation](https://github.com/vlang/v/blob/master/doc/docs.md) | The V Project | MIT |
| [V compiler and tools](https://github.com/vlang/v/tree/master/cmd) | The V Project | MIT |
| [V standard library](https://github.com/vlang/v/tree/master/vlib) | The V Project | MIT |
| [BeautifulRust](https://github.com/morganlinton/BeautifulRust) | Morgan Linton | MIT |

The skill text, the digests, the banner, and the scripts in this repository are MIT
licensed (see [LICENSE](LICENSE)).

## License

MIT. See [LICENSE](LICENSE).
