#!/bin/bash
# Light collection of the triangular bars (optical/sithopt): nominal optics for
# every geometry of the bar-size scan (420 mm long bars), optical systematics
# on a subset. Runs already done are skipped.
#
#   optical/run_optical.sh <out-dir> [threads]
set -e
out=$(realpath -m "$1"); nthr=${2:-$(nproc)}
src=$(realpath "$(dirname "$0")")
X=$src/build/sithopt
mkdir -p "$out"
cd "$out"

run() {   # run <tag> <points> <sithopt options...>
	local tag=$1 nev=$2; shift 2
	grep -q "collection efficiency" $tag.log 2>/dev/null && return
	"$X" --threads $nthr --events $nev --photons 50 --seed 1 --out $tag.root "$@" > $tag.log 2>&1
	echo "$tag: $(grep -o 'collection efficiency.*' $tag.log)"
}

for b in 20 30 42 60 84; do for h in 5 10 15 20 30; do
	run b${b}_h${h} 100000 --base $b --height $h --length 420
done; done

for g in "60 20" "42 30" "30 30" "20 20" "20 10"; do
	read -r b h <<< "$g"
	geo="--base $b --height $h --length 420"
	run b${b}_h${h}_rwrap0.90 50000 $geo --rwrap 0.90
	run b${b}_h${h}_rwrap0.98 50000 $geo --rwrap 0.98
	run b${b}_h${h}_rmask0.50 50000 $geo --rmask 0.50
	run b${b}_h${h}_rmask0.95 50000 $geo --rmask 0.95
	run b${b}_h${h}_att200    50000 $geo --att 200
	run b${b}_h${h}_sigma0.10 50000 $geo --sigma 0.10
	run b${b}_h${h}_npad1.00  50000 $geo --npad 1.0
	run b${b}_h${h}_both      50000 $geo --layout BOTH
done
