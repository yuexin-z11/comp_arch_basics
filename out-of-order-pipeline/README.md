# Out-of-Order Pipeline Simulator

Trace-driven timing simulator for a seven-stage CPU pipeline with register renaming, a reorder buffer (ROB), and in-order commit. It supports in-order scheduling and oldest-ready-first out-of-order scheduling, with one or two instruction lanes and configurable load latency.

The register alias table (RAT) maps architectural registers to the most recent uncommitted ROB producer. Issue resolves source dependencies before updating the destination mapping. Writeback wakes dependent instructions, and commit clears a mapping only when it still refers to the committing producer. The model assumes perfect branch prediction and independent memory operations; it tracks timing, not data values.

## Build and run

From this directory:

```bash
make -C src
./src/sim -pipewidth 2 -schedpolicy 1 -loadlatency 4 traces/sml.ptr.gz
```

`-pipewidth` selects one or two lanes for the supplied experiments. `-schedpolicy 0` schedules in order; `-schedpolicy 1` chooses the oldest ready instruction. `-loadlatency` sets the number of execution cycles for loads. Other instructions execute in one cycle.

## Verify

```bash
TERM=xterm bash scripts/runtests.sh
```

The script compares instruction count, cycle count, and CPI against 16 supplied reference cases for `gcc` and `sml`. Cases B1–B4 use one lane, while C1–C4 use two lanes; each group covers both scheduling policies and load latencies of one and four cycles. The `ref/` directory holds the supplied reference outputs and the `traces/` directory holds compressed input traces. Run the script from this project directory or pass its path from elsewhere; it changes to its own directory before running.

The original simulator framework and reference materials are retained so the comparisons remain reproducible.
