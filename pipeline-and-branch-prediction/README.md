# Pipeline and Branch Prediction

A trace-driven processor simulation lab covering pipeline timing, dependencies, forwarding, and branch prediction. This project corresponds to the original second lab.

**Status: Part A complete; Part B unfinished.** Dependency detection, ordered stalls, and forwarding are implemented. All six A1–A3 reference cases for `sml` and `gcc` match instruction counts, cycles, and CPI. Branch prediction still needs implementation. The files in `ref/` are supplied expected outputs.

## Architecture and intended experiments

The simulator models Fetch (IF), Decode (ID), Execute (EX), Memory Access (MA), and Write Back (WB). Each simulated cycle processes stages in reverse order, allowing earlier stages to observe stall information from later stages. The lab explores the following configurations:

| Script label | Width | Forwarding | Branch policy |
| --- | ---: | --- | --- |
| A1 | 1 | Disabled | Perfect |
| A2 | 2 | Disabled | Perfect |
| A3 | 2 | EX and MA | Perfect |
| B1 | 2 | EX and MA | Always taken |
| B2 | 2 | EX and MA | Gshare |

A1–A3 are implemented. B1–B2 require the unfinished branch predictor.

## Implementation map

- [`src/pipeline.cpp`](src/pipeline.cpp): Decode tracks the most recent older writer for each source register and condition codes, handles forwarding, and preserves instruction order during stalls. `pipe_check_bpred()` remains a Part B TODO.
- [`src/pipeline.h`](src/pipeline.h): pipeline state and latch definitions.
- [`src/bpred.cpp`](src/bpred.cpp): predictor constructor and update method are unimplemented; `predict()` returns a placeholder taken prediction. Predictor state and counters are not initialized by the current constructor.
- [`src/bpred.h`](src/bpred.h): perfect, always-taken, and Gshare policy definitions, plus saturating-counter helpers.
- [`src/sim.cpp`](src/sim.cpp): command-line parsing, trace input, simulation loop, and statistics.
- [`src/trace.h`](src/trace.h): pipeline trace record format.

## Build and run

From this project directory:

```bash
make -C src
./src/sim -pipewidth 1 traces/sml.ptr.gz
```

The small A1 trace produces 122 cycles and CPI 1.220, matching the reference.

After implementing Part B, an example two-wide configuration with forwarding and Gshare is:

```bash
./src/sim -pipewidth 2 -enablememfwd -enableexefwd -bpredpolicy 2 traces/gcc.ptr.gz
```

`-bpredpolicy` accepts `0` (perfect, default), `1` (always taken), and `2` (Gshare). Pipeline width defaults to 1; forwarding defaults to disabled. Non-perfect prediction results are unreliable until predictor initialization and behavior are implemented.

## Reference checks and batch runs

After completing the implementation, run the supplied comparison script:

```bash
mkdir -p results
TERM=xterm bash scripts/runtests.sh
```

The script compares instruction count, cycles, CPI, and applicable branch statistics against ten reference cases: A1–B2 for `sml` and `gcc`. Read the final pass count: the supplied script does not return a failing exit status merely because comparisons fail. `TERM=xterm` supports its terminal-color commands in environments without a configured terminal type.

Run the benchmark batch from `scripts/`:

```bash
mkdir -p results
cd scripts
sh runall.sh
```

This writes A1–B2 results for bzip2, gcc, libq, and mcf, and summarizes CPI and misprediction rate in `scripts/report.txt`.

## Files and results

- `traces/`: compressed `.ptr.gz` inputs; these use a different record format from the instruction-analysis project's `.otr.gz` files.
- `scripts/`: benchmark runner and reference-comparison script.
- `ref/results/`: supplied expected statistics.
- `ref/refoutput_gcc.pdf` and `ref/refoutput_sml.pdf`: supplied reference documents.
- `results/`: generated outputs, created before running the scripts.

No completed benchmark report is included for this project. A successful build alone does not validate the intended pipeline or branch-prediction behavior.

## Part A validation

All six supplied Part A cases match exactly:

| Case | Small cycles | GCC cycles | GCC CPI |
| --- | ---: | ---: | ---: |
| A1 | 122 | 11,412,411 | 1.141 |
| A2 | 77 | 6,720,461 | 0.672 |
| A3 | 57 | 5,206,917 | 0.521 |

The build completes with `-Wall` without warnings.
