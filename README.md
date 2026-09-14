# Computer Architecture Basics

Computer architecture labs for ECE 6100, organized by the concepts they explore.

| Project | Topics | Current status |
| --- | --- | --- |
| [Instruction Trace Analysis](instruction-trace-analysis/) | Dynamic instruction mix, cycles per instruction (CPI), unique instruction addresses | Analysis implemented; saved benchmark results included |
| [Pipeline and Branch Prediction](pipeline-and-branch-prediction/) | Five-stage pipelines, data hazards, forwarding, superscalar width, branch prediction | Part A implemented and all six reference cases pass; Part B branch prediction unfinished |

Each project includes its C++ source, compressed input traces, scripts, and a README with build and run instructions. The pipeline project also includes supplied reference outputs. Original course labels in scripts and result files are retained so the supplied tooling continues to work.

## Getting started

Use Linux or WSL with a C++11-capable `g++`, GNU Make, `gzip`/`gunzip`, and Bash. Clone the repository and follow either project README:

```bash
git clone https://github.com/yuexin-z11/Comp_Arch_Basics.git
cd Comp_Arch_Basics
make -C instruction-trace-analysis/src
./instruction-trace-analysis/src/sim instruction-trace-analysis/traces/libq.otr.gz
```

Compiled simulators and object files are excluded from version control. The original course framework, traces, and reference material are preserved alongside the lab work; Part A has been checked against the supplied reference results.
