# Contributing

Thanks for helping make Beautiful V better. Issues and pull requests are welcome. A few
conventions keep the skills consistent and trustworthy.

## The one rule: every rule needs a source

The skills are a synthesis of published V guidance, not a list of opinions. Each rule you
add or change should trace back to one of:

- `doc/docs.md` in the V repository
- the compiler, formatter, vetter, or test suite under `vlib/v/` and `cmd/`
- the standard library under `vlib/`
- an accepted issue or RFC in the V repository

Link it in your pull request. When two sources disagree, follow the compiler and say why.

## Good first contributions

- **A rule the skills get wrong or miss.** Open a "Propose a rule" issue with the source,
  a short example of code the agent writes today, and what it should write instead.
- **A review false positive.** If `v-review` flags something that is actually fine, open a
  "Review false positive" issue with the code and the finding. These become new eval cases.
- **A new eval case.** A realistic prompt, the input files it needs, and one assertion per
  file. See [Testing the skills](README.md#testing-the-skills) for the layout.
- **A broken install path.** Agents move their skill directories. If an install step in
  the README stopped working, a fix with a link to the agent's current docs is welcome.
- **A gate diagnostic that no longer fires.** Run `bash scripts/check-lints.sh`; a change
  in V's formatter or vetter should be reflected, not ignored.

## Making a change

1. **Edit the skill.** Each skill is `skills/<name>/SKILL.md`. Keep it under about 500
   lines; longer material belongs in `references/`. Explain why a rule matters, not just
   what it is. Agents follow reasoning better than commands.
2. **Compile your examples.** Every V code block in the skills and the README must compile
   with V 0.5.2 on stable. Fill in placeholders before checking. A fragment that cannot
   stand alone should be marked `v ignore`.
3. **Check gate changes.** If you touch `lints/`, run `bash scripts/check-lints.sh` and
   confirm the good fixture still passes and every expected diagnostic still fires.
4. **Run the evals if you changed behaviour.** From the skill's directory:

   ```bash
   claude plugin eval . --allow-tools Write,Edit,Bash --scaffold
   ```

   These are real model runs and are billed, so a maintainer can run them for you if you
   would rather not. Say in the pull request whether you ran them.
5. **Validate the plugin** with `claude plugin validate .` from the repo root, and the
   skills with `python3 scripts/check-skill.py`.
6. **Add a line to the Unreleased section of [CHANGELOG.md](CHANGELOG.md).**

## Style

- Write for an agent reading it mid-task: direct, imperative, one idea per bullet.
- Prefer a short code example to a paragraph of description.
- No Claude-specific instructions in the skills. They are standard Agent Skills and should
  work in any agent.
- Name the V mechanism precisely: `?T` and `!T`, `mut` receiver, `spawn` handle, `shared`
  lock. Do not use another language's vocabulary for a V feature.
- Do not claim a command works if you have not run it against V 0.5.2.

## License

By contributing, you agree your contribution is licensed under the MIT license in
[LICENSE](LICENSE).
