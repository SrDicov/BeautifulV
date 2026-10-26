#!/usr/bin/env bash
# Prove the gate in lints/gate.sh still catches what it claims: the good
# fixture passes, and the bad fixture trips every diagnostic the README lists.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GATE="$ROOT/lints/gate.sh"
GOOD="$ROOT/lints/check/good.v"
BAD="$ROOT/lints/check/bad.v"

if ! command -v v >/dev/null 2>&1; then
	echo "check-lints: v not found on PATH" >&2
	exit 1
fi

if ! "$GATE" "$GOOD" >/tmp/beautifulv-gate-good.log 2>&1; then
	echo "check-lints: the good fixture did not pass the gate" >&2
	cat /tmp/beautifulv-gate-good.log >&2
	exit 1
fi

set +e
"$GATE" "$BAD" >/tmp/beautifulv-gate-bad.log 2>&1
STATUS=$?
set -e

if [ "$STATUS" -eq 0 ]; then
	echo "check-lints: the bad fixture passed the gate" >&2
	exit 1
fi

OUTPUT="$(cat /tmp/beautifulv-gate-bad.log)"
FAILED=0

check() {
	if ! grep -Fq -- "$1" <<<"$OUTPUT"; then
		echo "check-lints: expected diagnostic missing: $1" >&2
		FAILED=1
	fi
}

check "is not vfmt'ed"
check "Looks like you are using spaces for indentation."
check "Looks like you have trailing whitespace."
check "Function documentation seems to be missing"
check 'Use `x == 1` instead of `x in [1]`'

if [ "$FAILED" -ne 0 ]; then
	echo >&2
	echo "gate output was:" >&2
	echo "$OUTPUT" >&2
	exit 1
fi

echo "gate caught all 5 expected diagnostics; good fixture passes"
