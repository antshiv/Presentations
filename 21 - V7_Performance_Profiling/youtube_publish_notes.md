# YouTube Publish Notes - CPU LLM Season 2

## Suggested Title
How to Profile LLM Inference on CPU on Linux - perf, FlameGraph, VTune, Advisor (CPU LLM Season 2)

## Short Description
In this episode, I walk through a practical Linux workflow to profile LLM inference on CPU using real tooling: `perf stat`, FlameGraph, Intel VTune, and Intel Advisor.

The full flow is runbook-aligned:
- Generate the inference run and compiled artifacts
- Regenerate `ir_report.html`
- Analyze decode vs prefill with mode-specific profiling
- Classify bottlenecks before making optimization claims

Runbook:
https://antshiv.github.io/C-Kernel-Engine/v7-runbook.html

IR report used in this recording:
`/home/antshiv/.cache/ck-engine-v7/models/Qwen--Qwen2-0.5B-Instruct-GGUF/ir_report.html`

## Pinned Comment
Runbook + workflow used in this video:
https://antshiv.github.io/C-Kernel-Engine/v7-runbook.html

This recording focuses on inference profiling flow on Linux:
`perf stat` -> FlameGraph (decode + prefill) -> VTune -> Advisor.

If you run this on your machine, keep model/prompt/token/thread settings fixed so your deltas are trustworthy.

Thumbnail note: created with Google Gemini (Nano Banana) and edited for final video layout.
