---
tags: [v-best-practices]
max_turns: 20
timeout_seconds: 900
---

Write a V command-line tool that summarises a log file. Each line looks like:

```
2026-01-02T03:04:05Z INFO  started
2026-01-02T03:04:06Z WARN  retrying
2026-01-02T03:04:07Z ERROR failed
```

Requirements:

- Usage: `logsum <path>`.
- Count lines per level (`INFO`, `WARN`, `ERROR`, `DEBUG`, and any other level).
- Skip malformed lines (missing timestamp, level, or message) and report the count.
- Print one level per line, sorted by level name, then a `skipped: N` line.
- A missing or unreadable file prints an error naming the path to stderr and exits 1.
- Never panic on bad input.
- Put the counting and formatting in functions that can be unit-tested, and add
  `_test.v` tests for them.

Use V 0.5.x. Handle the `os.read_file` result; do not use `or { panic(err) }`.
