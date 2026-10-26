#!/usr/bin/env bash
# The style gate for a V project: formatting and the vet diagnostics that
# matter. Copy this file into your repository, or call it from CI.
#
# Usage: gate.sh [file-or-directory ...]   (default: the current directory)
set -uo pipefail

if ! command -v v >/dev/null 2>&1; then
	echo "gate: v not found on PATH" >&2
	exit 1
fi

if [ "$#" -eq 0 ]; then
	set -- .
fi

STATUS=0

# v fmt -verify exits 1 if any file would change.
v fmt -verify "$@" || STATUS=1

# v vet -W turns missing docs, space indentation, trailing whitespace, and the
# single-element `in` check into errors.
v vet -W "$@" || STATUS=1

exit "$STATUS"
