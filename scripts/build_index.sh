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

find_entry_file() {
  local dir="$1"
  local full_dir="$ROOT_DIR/$dir"
  local file
  local preferred=(
    "index.html"
    "demo.html"
    "presentation.html"
    "main.html"
  )

  for file in "${preferred[@]}"; do
    if [[ -f "$full_dir/$file" ]]; then
      printf '%s' "$file"
      return 0
    fi
  done

  mapfile -t candidates < <(
    find "$full_dir" -maxdepth 1 -type f -name '*.html' -printf '%f\n' \
      | grep -Evi '(notes|speaker|aside|draft|backup)' \
      | sort
  )

  if ((${#candidates[@]} > 0)); then
    printf '%s' "${candidates[0]}"
    return 0
  fi

  mapfile -t candidates < <(
    find "$full_dir" -maxdepth 1 -type f -name '*.html' -printf '%f\n' | sort
  )

  if ((${#candidates[@]} > 0)); then
    printf '%s' "${candidates[0]}"
    return 0
  fi

  return 1
}

describe_presentation() {
  local dir="$1"
  case "$dir" in
    "08 - hetrogenous") echo "Explains heterogeneous execution and how splitting work across compute units improves throughput." ;;
    "09 - A.I_training") echo "Walks through LLM training flow: pretraining, supervised fine-tuning, and reinforcement learning stages." ;;
    "10 - LLM_memory_layout") echo "Covers memory layout decisions, cache behavior, and why layout affects token throughput." ;;
    "11 - LLM_parallel_memory") echo "Shows parallel memory access patterns and alignment strategies for model inference." ;;
    "12 - AdvancedMemory") echo "Dives into advanced memory topics including NUMA, prefetching, and practical optimization tradeoffs." ;;
    "13 - GEMM") echo "Breaks down GEMM fundamentals and optimization ideas like tiling, blocking, and vectorization." ;;
    "14 - LayerNorm") echo "Explains LayerNorm math, where it sits in transformers, and implementation-level performance considerations." ;;
    "15 - Attention") echo "Presents attention mechanism internals from intuition to matrix math and dataflow behavior." ;;
    "16 - MLP_Inference") echo "Focuses on MLP inference path, GELU behavior, and compute vs memory bottlenecks." ;;
    "17 - GPT-2 Backprop") echo "Step-by-step walkthrough of GPT-2 backpropagation and gradient flow across layers." ;;
    "18 - GPT2 What I learnt") echo "Summarizes practical lessons learned from building and studying GPT-2 style models." ;;
    "19 - C-Kernel-Engine") echo "Introduces C-Kernel-Engine architecture and how core kernels map to model operations." ;;
    "20 - Qwen_Optimization") echo "Details optimizations applied to Qwen-style model inference and runtime efficiency." ;;
    "21 - V7_Performance_Profiling") echo "Profiling runbook for v7: measuring hotspots, interpreting traces, and prioritizing fixes." ;;
    "22 - V7_SVG_Training_Runbook") echo "Operational runbook for v7 SVG training: setup, execution flow, and validation checkpoints." ;;
    *) echo "Presentation deck." ;;
  esac
}

mapfile -t presentation_dirs < <(
  cd "$ROOT_DIR"
  find . -mindepth 1 -maxdepth 1 -type d \
    | sed 's#^\./##' \
    | grep -E '^(0[8-9]|1[0-9]|2[0-2]) - ' \
    | sort -V
)

declare -a pages=()
for dir in "${presentation_dirs[@]}"; do
  if entry_file="$(find_entry_file "$dir")"; then
    pages+=("$dir/$entry_file")
  else
    echo "Warning: no HTML file found for '$dir'" >&2
  fi
done

{
  cat <<'HTML_HEAD'
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Presentations</title>
  <style>
    body {
      margin: 0;
      min-height: 100vh;
      color: #fff;
      background:
        radial-gradient(circle at 20% 0%, rgba(66, 175, 250, 0.20), transparent 38%),
        radial-gradient(circle at 100% 100%, rgba(255, 45, 96, 0.12), transparent 35%),
        #191919;
      font-family: "Source Sans Pro", Helvetica, sans-serif;
    }
    .hub {
      max-width: 1200px;
      margin: 0 auto;
      padding: 48px 20px 64px;
    }
    .title {
      margin: 0;
      font-size: clamp(2.1rem, 4vw, 3rem);
      letter-spacing: 0.02em;
    }
    .subtitle {
      margin: 10px 0 26px;
      color: rgba(255, 255, 255, 0.75);
      font-size: 1.1rem;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      border: 1px solid rgba(255, 255, 255, 0.18);
      border-radius: 12px;
      overflow: hidden;
      background: rgba(255, 255, 255, 0.04);
    }
    thead th {
      text-align: left;
      padding: 12px 14px;
      background: rgba(66, 175, 250, 0.20);
      color: #dff3ff;
      font-size: 0.95rem;
      letter-spacing: 0.03em;
    }
    tbody td {
      padding: 12px 14px;
      border-top: 1px solid rgba(255, 255, 255, 0.12);
      vertical-align: top;
    }
    tbody tr:nth-child(even) {
      background: rgba(255, 255, 255, 0.03);
    }
    .deck {
      font-weight: 700;
      white-space: nowrap;
    }
    .open-link {
      color: #79d0ff;
      text-decoration: none;
      font-weight: 600;
    }
    .open-link:hover,
    .open-link:focus-visible {
      text-decoration: underline;
      outline: none;
    }
    .entry-file {
      color: rgba(255, 255, 255, 0.72);
      font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
      font-size: 0.82rem;
    }
    .description {
      color: rgba(255, 255, 255, 0.88);
      line-height: 1.35;
    }
    @media (max-width: 860px) {
      table, thead, tbody, tr, th, td {
        display: block;
      }
      thead {
        display: none;
      }
      tbody tr {
        margin-bottom: 10px;
        border: 1px solid rgba(255, 255, 255, 0.12);
        border-radius: 10px;
        overflow: hidden;
      }
      tbody td {
        border-top: none;
        border-bottom: 1px solid rgba(255, 255, 255, 0.08);
      }
      tbody td:last-child {
        border-bottom: none;
      }
    }
    .count {
      margin-top: 18px;
      color: rgba(255, 255, 255, 0.66);
      font-size: 0.95rem;
    }
  </style>
</head>
<body>
  <main class="hub">
  <h1 class="title">Presentations 08-22</h1>
  <p class="subtitle">Direct links plus a short explanation of what each deck covers.</p>
  <table>
    <thead>
      <tr>
        <th>Presentation</th>
        <th>What It Covers</th>
        <th>Open</th>
      </tr>
    </thead>
    <tbody>
HTML_HEAD

  for page in "${pages[@]}"; do
    label="${page%/*}"
    entry="${page##*/}"
    description="$(describe_presentation "$label")"
    href="$(url_encode "$page")"
    printf '      <tr><td class="deck">%s</td><td class="description">%s</td><td><a class="open-link" href="./%s">Open</a><br><span class="entry-file">%s</span></td></tr>\n' "$label" "$description" "$href" "$entry"
  done

  cat <<HTML_TAIL
    </tbody>
  </table>
  <p class="count">Total presentations linked: ${#pages[@]}</p>
  </main>
</body>
</html>
HTML_TAIL
} > "$OUTPUT_FILE"

echo "Generated $OUTPUT_FILE with ${#pages[@]} presentation links."
