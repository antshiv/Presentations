# Recording Artifacts - V7 Performance Profiling

This file captures the commands and artifact paths used for the recorded workflow.

## Environment
- Repo: `/home/antshiv/Workspace/C-Kernel-Engine`
- Run directory (example used):
  - `/home/antshiv/.cache/ck-engine-v7/models/Qwen--Qwen2-0.5B-Instruct-GGUF`
- Alternate run referenced during prep:
  - `/home/antshiv/.cache/ck-engine-v7/models/Qwen--Qwen3-0.6B-GGUF`

## Kernel profiling gates (Linux)
```bash
sudo sysctl -w kernel.perf_event_paranoid=1 kernel.kptr_restrict=0
```

## Step 8 - Generate/compile run artifacts
```bash
python3 version/v7/scripts/ck_run_v7.py run \
  hf://Qwen/Qwen3-0.6B-GGUF/Qwen3-0.6B-Q8_0.gguf \
  --context-len 1024 --force-compile --force-convert \
  --generate-visualizer
```

## Step 8.1 - Regenerate visualizer from run dir
```bash
python3 version/v7/tools/open_ir_visualizer.py \
  --generate --run "$RUN" --html-only
```

## Deterministic timing baseline
```bash
make --no-print-directory ck-cli-v7
./build/ck-cli-v7 "$RUN/libmodel.so" "$RUN/weights.bump" \
  --prompt "<svg" --max-tokens 80 --timing --quiet-output --no-chat-template
```

## FlameGraph (decode + prefill)
```bash
make --no-print-directory profile-v7-flamegraph-decode \
  V7_MODEL="$RUN" V7_PERF_RUNTIME=cli V7_CHAT_TEMPLATE=none V7_FORCE_COMPILE=0

make --no-print-directory profile-v7-flamegraph-prefill \
  V7_MODEL="$RUN" V7_PERF_RUNTIME=cli V7_CHAT_TEMPLATE=none V7_FORCE_COMPILE=0
```

## perf stat (baseline counters)
```bash
make --no-print-directory profile-v7-perf-stat \
  V7_MODEL="$RUN" V7_PERF_RUNTIME=cli V7_CHAT_TEMPLATE=none
```

## VTune
```bash
# Fast pass
make --no-print-directory profile-v7-vtune \
  V7_MODEL="$RUN" V7_PERF_RUNTIME=cli V7_CHAT_TEMPLATE=none \
  V7_WITH_VTUNE=1 V7_VTUNE_DEEP=0

# Deep pass (memory/uarch)
make --no-print-directory profile-v7-vtune \
  V7_MODEL="$RUN" V7_PERF_RUNTIME=cli V7_CHAT_TEMPLATE=none \
  V7_WITH_VTUNE=1 V7_VTUNE_DEEP=1
```

## Advisor
```bash
make --no-print-directory profile-v7-advisor \
  V7_MODEL="$RUN" V7_PERF_RUNTIME=cli V7_CHAT_TEMPLATE=none \
  V7_WITH_ADVISOR=1
```

## Optional safety checks
```bash
make --no-print-directory profile-v7-prepare-runtime \
  V7_MODEL="$RUN" V7_PREP_WITH_PYTHON=1 V7_FORCE_COMPILE=1

make --no-print-directory profile-v7-cachegrind \
  V7_MODEL="$RUN" V7_PERF_RUNTIME=cli V7_CHAT_TEMPLATE=none
```

## Key report/artifact paths
- IR report:
  - `$RUN/ir_report.html`
- FlameGraph manifest:
  - `$RUN/flamegraph_manifest.json`
- Profile summary:
  - `$RUN/profile_summary.json`
- perf summary:
  - `$RUN/perf_stat_summary.json`
- VTune summary:
  - `$RUN/vtune_summary.json`
- Advisor summary:
  - `$RUN/advisor_summary.json`
- FlameGraph SVGs:
  - `build/flamegraph_v7.svg`
  - `build/flamegraph_v7_prefill.svg`

## Notes
- Keep prompt/token/thread settings fixed across all runs.
- Decode and prefill must both be captured for mode-correct flamegraph switching in the IR visualizer.
