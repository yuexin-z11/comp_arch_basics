# Out-of-Order Pipeline Simulator

Trace-driven seven-stage CPU timing simulator built on the supplied ECE 6100 framework. The project explores register renaming, a reorder buffer (ROB), in-order commit, and in-order versus oldest-ready-first scheduling.

## Current status

The register alias table and ROB functions have been filled in and compile independently with C++11 and `-Wall`. The pipeline's issue, schedule, writeback, and commit stages still contain starter TODOs. The simulator's reference results are included for future verification; passing them is not yet claimed.

## Build and verify current work

From this directory:

```bash
g++ -std=c++11 -Wall -c src/rat.cpp -o /tmp/rat.o
g++ -std=c++11 -Wall -c src/rob.cpp -o /tmp/rob.o
```

After the pipeline stages are implemented, build and compare against the supplied reference cases:

```bash
make -C src
TERM=xterm bash scripts/runtests.sh
```

The reference set covers one- and two-wide pipelines, both scheduling policies, and load latencies of one and four cycles. The `src/` directory retains the supplied interfaces and framework; `traces/` and `ref/` hold the supplied inputs and expected results.
