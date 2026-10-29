#!/bin/bash
# Analyse a run_scan.sh output directory with the ORIGINAL ROOT/PyROOT tools
# (output/Simulation_runner.cpp + output/Peak_fitter.py), configured with the
# same sed edits run.sh used to apply.
#   scripts/analyse_original.sh <scan-dir> <theta|tb> <n-threads-used>
set -e
dir=$(realpath "$1"); mode=$2; nth=$3
src=$(realpath "$(dirname "$0")/..")
cd "$dir/output"
cp "$src/output/Simulation_runner.cpp" "$src/output/Peak_fitter.py" .
if [ $mode = theta ]; then cfg=PERPENDICULAR; nr=26; setting=THETA; else cfg=PARALLEL; nr=7; setting=G4; fi
sed -i "s/^const int n_runs.*/const int n_runs = $nr;/; s/^const int nth.*/const int nth = $nth;/; s/^bool configuration.*/bool configuration = $cfg;/" Simulation_runner.cpp
sed -i "s/^n_runs      = .*/n_runs      = $nr/; s/^configuration.*/configuration = \"$cfg\"/; s/^setting.*/setting = \"$setting\"/" Peak_fitter.py
sed -i 's/^import time$/import time\nrt.gROOT.SetBatch(True)/' Peak_fitter.py
root -l -b -q Simulation_runner.cpp > simulation_runner.log 2>&1
tail -3 simulation_runner.log
echo | python Peak_fitter.py > peak_fitter.log 2>&1 || true
grep -E "AVERAGE|CALIBRATION|CHI/NDF" peak_fitter.log
