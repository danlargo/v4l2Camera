#!/usr/bin/env bash
set -euo pipefail

HEADER_FILE="${1:-v4l2camera.h}"

if [[ ! -f "$HEADER_FILE" ]]; then
  echo "Error: $HEADER_FILE not found" >&2
  exit 1
fi

# Find current: static const int s_build = <number>;
current=$(grep -E 's_build[[:space:]]*=[[:space:]]*[0-9]+' "$HEADER_FILE" | head -n1 | sed -E 's/.*s_build[[:space:]]*=[[:space:]]*([0-9]+).*/\1/')
if [[ -z "${current:-}" ]]; then
  echo "Error: could not find 's_build = <number>;' in $HEADER_FILE" >&2
  exit 1
fi

next=$(( current + 1 ))

# Replace only the first occurrence, preserve spacing and semicolon
sed -i -E "0,/s_build([[:space:]]*=[[:space:]]*)[0-9]+/s//s_build\1${next}/" "$HEADER_FILE"

echo "s_build: ${current} -> ${next}"