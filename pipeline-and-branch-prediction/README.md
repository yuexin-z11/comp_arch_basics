# Pipeline and Branch Prediction

Trace-driven five-stage pipeline simulator for ECE 6100 Lab 2. It implements
the Part A dependency/forwarding behavior and the Part B always-taken and
12-bit Gshare branch predictors.

## Build

From this project directory:

```bash
make -C src
```

## Run

```bash
./src/sim [options] traces/<trace>.ptr.gz
```

Useful options are `-pipewidth <width>`, `-enablememfwd`, `-enableexefwd`, and
`-bpredpolicy <policy>`, where policy `0` is perfect, `1` is always taken, and
`2` is Gshare. For example:

```bash
./src/sim -pipewidth 2 -enablememfwd -enableexefwd -bpredpolicy 2 traces/sml.ptr.gz
```

## Verify

```bash
mkdir -p results
TERM=xterm bash scripts/runtests.sh
```

All ten supplied A1-A3 and B1-B2 reference cases for `sml` and `gcc` pass.

## Create the Part B submission

From the original `Lab_2` directory:

```bash
tar -cvzf src_B.tar.gz src
```

Submit `src_B.tar.gz`; do not include traces.
