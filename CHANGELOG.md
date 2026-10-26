# Changelog

All notable changes to this project are listed here. Versions follow
[Semantic Versioning](https://semver.org/), and the version lives in
`.claude-plugin/plugin.json`.

## [Unreleased]

## [1.0.0] - 2026-10-26

The first release: a V-focused fork of BeautifulRust.

### Added

- **`v-best-practices`**, a writing skill covering V's mutability rules, Option and
  Result types, sum types, interfaces, generics, memory modes, concurrency, testing,
  documentation, and performance tuning.
- **`v-review`**, a review skill that runs the V toolchain and writes a prioritized
  report where every finding cites a location and the rule it breaks.
- **`v-port`**, a porting skill for C, C++, Python, Go, JavaScript, and TypeScript,
  with the original tests ported first.
- **`lints/gate.sh`**, a drop-in `v fmt -verify` plus `v vet -W` gate, with a fixture
  that proves each diagnostic still fires.
- **`scripts/check-skill.py`**, a standard-library-only validator for skill
  frontmatter, links, eval cases, and the plugin manifests.

[Unreleased]: https://github.com/SrDicov/BeautifulV/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/SrDicov/BeautifulV/releases/tag/v1.0.0
