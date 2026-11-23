# CMP Memory System Simulator

A trace-driven memory hierarchy simulator built on the supplied ECE 6100 framework. It models private L1 instruction and data caches, a shared L2 cache, and a 16-bank DRAM with row buffers. It starts as a single-core system and is extended to two cores sharing the L2.

## Features

| Mode | Part | Model |
| --- | --- | --- |
| `-mode 1` | A | Standalone data cache with configurable size, associativity, line size, and LRU or random replacement |
| `-mode 2` | B | L1 I and D caches, a unified L2, and DRAM with a fixed 100-cycle latency. Write-back, allocate-on-write-miss |
| `-mode 3` | C | Row-buffer DRAM: 16 banks, open-page (`-dram_policy 0`) or close-page (`-dram_policy 1`) |
| `-mode 4` | D | Two cores with per-core L1s and a shared LRU L2, using virtual-to-physical translation |
| `-mode 4 -L2repl 2` | E | Static way partitioning of the L2 (`-SWP_core0ways N`) |
| `-mode 4 -L2repl 3` | F | Dynamic way partitioning of the L2 (see `DWP_report.txt`) |

Status of each part is tracked in [PROGRESS.md](PROGRESS.md).

## Build and verify

From this directory:

```bash
make -C src
TERM=xterm bash scripts/runtests.sh      # diff against ref/results
```

Run a single configuration:

```bash
./src/sim -mode 3 -L2sizeKB 1024 -dram_policy 0 traces/libq.mtr.gz
./src/sim -mode 4 traces/bzip2.mtr.gz traces/libq.mtr.gz
```

Generate all results and package the submissions:

```bash
(cd scripts && bash runall.sh)           # writes results/*.res
bash scripts/package.sh ABC              # or DEF; output in submission/
```

The `src/` directory keeps the supplied interfaces. `traces/` and `ref/` hold the supplied inputs and expected results.
