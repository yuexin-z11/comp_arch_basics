#!/bin/bash -e
# Build the course submission tarballs into ../submission/.
#   bash scripts/package.sh ABC   -> src_ABC.tar.gz, results_ABC.tar.gz
#   bash scripts/package.sh DEF   -> src_DEF.tar.gz, results_DEF.tar.gz, DWP_report.txt
# Run scripts/runall.sh first so results/ is up to date.

cd "$(dirname "$0")/.."
phase="${1:?usage: package.sh ABC|DEF}"
out=submission
mkdir -p "$out"

make -C src clean > /dev/null

case "$phase" in
    ABC) parts=(A B C) ;;
    DEF) parts=(D E F) ;;
    *) echo "unknown phase: $phase" >&2; exit 1 ;;
esac

tar -czf "$out/src_${phase}.tar.gz" src

shopt -s nullglob
res=()
for p in "${parts[@]}"; do res+=(results/"$p".*.res); done
if [[ ${#res[@]} -eq 0 ]]; then
    echo "no results for parts ${parts[*]}; run scripts/runall.sh first" >&2
    exit 1
fi
tar -czf "$out/results_${phase}.tar.gz" "${res[@]}"

if [[ "$phase" == DEF ]]; then
    cp DWP_report.txt "$out/"
fi

echo "Packaged ${#res[@]} result files:"
ls -l "$out"
