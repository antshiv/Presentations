#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUTPUT_FILE="$ROOT_DIR/index.html"

url_encode() {
  local s="$1"
  local out=""
  local i ch
  for ((i=0; i<${#s}; i++)); do
    ch="${s:i:1}"
    case "$ch" in
      [a-zA-Z0-9.~_-]|"/") out+="$ch" ;;
      " ") out+='%20' ;;
      *) printf -v out '%s%%%02X' "$out" "'$ch" ;;
    esac
  done
  printf '%s' "$out"
}

mapfile -t pages < <(
  cd "$ROOT_DIR"
  find . -mindepth 2 -maxdepth 2 -type f -name index.html \
    | sed 's#^\./##' \
    | grep -v '^reveal\.js/index\.html$' \
    | sort
)

{
  cat <<'HTML_HEAD'
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Presentations</title>
  <style>
    body { font-family: sans-serif; max-width: 900px; margin: 32px auto; padding: 0 16px; }
    h1 { margin-bottom: 8px; }
    p { color: #444; }
    ul { padding-left: 20px; }
    li { margin: 8px 0; }
    a { text-decoration: none; }
    a:hover { text-decoration: underline; }
  </style>
</head>
<body>
  <h1>Presentations</h1>
  <p>Direct links to all presentation decks in this repository.</p>
  <ul>
HTML_HEAD

  for page in "${pages[@]}"; do
    label="${page%/index.html}"
    href="$(url_encode "$page")"
    printf '    <li><a href="./%s">%s</a></li>\n' "$href" "$label"
  done

  cat <<'HTML_TAIL'
  </ul>
</body>
</html>
HTML_TAIL
} > "$OUTPUT_FILE"

echo "Generated $OUTPUT_FILE with ${#pages[@]} presentation links."
