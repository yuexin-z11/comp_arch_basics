# Instruction Trace Analysis

Analyze a dynamic CPU instruction trace to measure instruction mix, estimate cycles per instruction (CPI), and count distinct instruction addresses. This project corresponds to the original first lab.

## Implementation

[`src/studentwork.cpp`](src/studentwork.cpp) implements `analyze_trace_record()`:

- Count each instruction by operation type: ALU, load, store, conditional branch, or other.
- Accumulate cycles using the fixed per-instruction cost below.
- Track instruction addresses in an `std::unordered_set` and count each address once.

The driver in [`src/sim.cpp`](src/sim.cpp) reads compressed traces and maintains the total dynamic instruction count. [`src/trace.h`](src/trace.h) defines the record format.

| Operation | Modeled cycles |
| --- | ---: |
| ALU | 1 |
| Load | 2 |
| Store | 2 |
| Conditional branch | 3 |
| Other | 1 |

CPI = total modeled cycles / dynamic instruction count. Instruction mix percentages use the same dynamic count as their denominator. Unique PCs measure the number of distinct instruction addresses, not a footprint in bytes. This is a fixed-cost model, not a pipeline timing simulation.

## Build and run

From this project directory:

```bash
make -C src
./src/sim traces/libq.otr.gz
```

To rerun all four benchmarks and regenerate the supplied report:

```bash
cd scripts
sh runall.sh
```

Run the batch script from `scripts/` because it uses relative paths. It writes `results/Lab1.<benchmark>.res` and `scripts/report.txt`. The bundled `genreport.ecelinsrv7` is a course-provided Linux x86-64 executable; if it cannot run on your platform, the simulator still produces the individual `.res` files. No source for that report generator is included.

## Saved results

These values come from the included [`scripts/report.txt`](scripts/report.txt) and [`results/`](results/). Percentages are rounded to three decimals.

| Metric | libq | bzip2 | mcf | gcc |
| --- | ---: | ---: | ---: | ---: |
| ALU (%) | 36.364 | 9.176 | 16.539 | 12.520 |
| Load (%) | 18.182 | 49.753 | 47.362 | 41.651 |
| Store (%) | 18.182 | 11.598 | 11.280 | 24.412 |
| Conditional branch (%) | 9.091 | 10.462 | 15.500 | 9.652 |
| Other (%) | 18.182 | 19.010 | 9.319 | 11.765 |
| CPI | 1.545 | 1.823 | 1.896 | 1.854 |
| Unique PCs | 9 | 764 | 524 | 820 |

Under this model, loads/stores and conditional branches increase CPI above the one-cycle baseline. The `mcf` trace has the highest modeled CPI of these four traces.

## Files

- `src/`: trace reader, analysis implementation, record definition, and Makefile.
- `traces/`: compressed `.otr.gz` inputs for libq, bzip2, mcf, and gcc.
- `scripts/`: batch runner, supplied report generator, and saved report.
- `results/`: saved simulator output for each benchmark.
