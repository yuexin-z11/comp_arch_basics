# Out-of-Order Pipeline Simulator

Trace-driven seven-stage CPU timing simulator built on the supplied ECE 6100 framework. The project explores register renaming, a reorder buffer (ROB), in-order commit, and in-order versus oldest-ready-first scheduling.

## Features

- Register renaming through a register alias table and a circular reorder buffer.
- In-order and oldest-ready-first out-of-order scheduling.
- In-order commit with one- or two-wide issue, scheduling, and commit.
- Configurable load execution latency.

## Build and verify

From this directory:

```bash
make -C src
TERM=xterm bash scripts/runtests.sh
```

Run a trace with a selected width, scheduling policy, and load latency:

```bash
./src/sim -pipewidth 2 -schedpolicy 1 -loadlatency 4 traces/sml.ptr.gz
```

Scheduling policy `0` is in order; `1` is oldest-ready-first out of order. The supplied reference set covers both policies, widths 1 and 2, and load latencies 1 and 4. All 16 reference cases pass. The `src/` directory retains the supplied interfaces and framework; `traces/` and `ref/` hold the supplied inputs and expected results.
