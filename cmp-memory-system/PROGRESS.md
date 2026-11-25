# Progress Tracker

Deadlines: **A+B+C** by Wed Nov 25, 2026, 11:59 pm ET. **D (+E+F)** by Fri Dec 4, 2026, 11:59 pm ET.

Workflow: feature branches for A-D were merged with merge commits to preserve their commits. E is integrated directly on `main`.

## Parts

| Part | Pts | Branch | Files | Ref tests | Status |
| --- | --- | --- | --- | --- | --- |
| Skeleton | – | `lab4/skeleton` | structs, tooling, docs | – | Merged |
| A: cache | 3 | `feature/set-associative-cache` | `cache.cpp` | `A.libq` | Verified |
| B: multi-level | 2 | `feature/multi-level-cache` | `memsys.cpp`, `dram.cpp` (fixed 100) | `B.S1MB.libq` | Verified |
| C: DRAM row buffer | 3 | `feature/dram-row-buffer` | `dram.cpp` | `C.S1MB.{OP,CP}.libq` | Verified |
| D: two cores | 2 | `feature/two-core-memory-system` | `memsys.cpp` | `D.mix1` | Verified |
| E: SWP (EC) | +2 | `main` | `cache.cpp` | `E.Q{1,2,3}.mix1` | Verified |
| F: DWP (EC) | +3 | `lab4/part-f` | `cache.{h,cpp}`, `DWP_report.txt` | none (beat D/E miss % by >1% on one mix) | Todo |

## What was added on top of the supplied framework

Commit `Import supplied CMP memory system framework` contains the framework exactly as shipped. Everything else is listed here.

### Files added
- `README.md`: project overview and how to build, test and run.
- `PROGRESS.md`: this file.
- `DWP_report.txt`: template for the Part F report.
- `scripts/package.sh`: builds `submission/src_{ABC,DEF}.tar.gz`, `results_{ABC,DEF}.tar.gz` and copies `DWP_report.txt`.
- `results/.gitkeep`: keeps the output directory that `runall.sh` writes to.

### Supplied files changed (skeleton only, no logic)
- `src/cache.h`: added `CacheLine` and `CacheSet` structs and `Cache` fields (`sets`, `num_sets`, `num_ways`, `replacement_policy`, `last_evicted_line`). Moved the `ReplacementPolicy` enum above `Cache` so the struct can use it.
- `src/dram.h`: added the `RowbufEntry` struct, the `DRAM_NUM_BANKS` constant and `DRAM::perbank_row_buf[16]`.
- `scripts/*.sh`: marked executable (git mode only).

### Added TODO comments
The original course comments are plain `// TODO:`. Every TODO added here is tagged `TODO(lab4-<part>)`, so `grep -rn "TODO(lab4" .` lists only these.

| File | Function / location | Tag | Note |
| --- | --- | --- | --- |
| `src/cache.h` | `struct Cache` | F | dynamic partitioning state |
| `src/cache.cpp` | `cache_new` | A | calloc sets, derive `num_sets` |
| `src/cache.cpp` | `cache_access` | A | tag/index split, LRU timestamp, stats |
| `src/cache.cpp` | `cache_install` | A | `last_evicted_line`, dirty evicts |
| `src/cache.cpp` | `cache_find_victim` | A, E, F | invalid → LRU/RAND; SWP quota; DWP targets |
| `src/dram.cpp` | `dram_new` | B | calloc |
| `src/dram.cpp` | `dram_access` | B, C | fixed 100 in mode B; delegate in C+ |
| `src/dram.cpp` | `dram_access_mode_CDEF` | C | bank/row map, open/close page |
| `src/memsys.cpp` | `memsys_access_modeBC` | B | L1 → L2, L1 writebacks |
| `src/memsys.cpp` | `memsys_l2_access` | B | L2 miss → DRAM, L2 writebacks |
| `src/memsys.cpp` | `memsys_access_modeDEF` | D | VPN→PFN, per-core L1s |
| `DWP_report.txt` | top | F | write report |

Remove each tag once its part is implemented and verified.

## Notes and gotchas
- **Mix naming:** the handout text says Mix1 = libq+bzip2, Mix2 = libq+lbm, Mix3 = bzip2+lbm. `runall.sh` (whose output names get graded) uses mix1 = bzip2+libq, mix2 = bzip2+lbm, mix3 = lbm+libq. We follow `runall.sh`.
- **Handout typo:** the Phase 2 steps say `src_ABC.tar.gz` / `results_ABC.tar.gz`, but the heading says `src_DEF` / `results_DEF`. `package.sh` uses the `_DEF` names.
- Part F lines in `runall.sh` remain commented out until that policy is implemented.
- `dram.cpp` has its own `NUM_BANKS 16`; keep it consistent with `DRAM_NUM_BANKS` in `dram.h` (add a `static_assert` in Part C).
- L2 is 1 MB, 16-way by default, so SWP 50% = `-SWP_core0ways 8`.
- Random replacement may differ slightly from the reference (FAQ 3); only LRU is checked by `runtests.sh`.
- Do not change the print functions' output; grading diffs `CYCLES|CORE_|MEMSYS_|ICACHE_|DCACHE_|L2CACHE_|DRAM_` lines.
