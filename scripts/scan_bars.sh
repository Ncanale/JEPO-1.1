#!/bin/bash
# Phase 3: resolution vs triangular-bar size at fixed active area (420 mm).
#   1. calibrate the PHOTOSTAT smearing on the test-beam setup, so that it gives
#      the same offset distributions as the CONSTANT 0.22 tuning to the 2018 data
#   2. Geant4 overlap check of every geometry
#   3. one 'uniform' run per geometry and smearing model, analysed with sith_reco.py
#
#   scripts/scan_bars.sh <out-dir> [events-per-point]
set -e
out=$(realpath -m "$1"); nev=${2:-500000}
src=$(realpath "$(dirname "$0")/..")
build=$src/build
mkdir -p "$out"
cd "$out"
BASES=(20 30 42 60 84)          # mm, 420 mm / base bars per row
HEIGHTS=(5 10 15 20 30)         # mm
EREF=7.0                        # MeV, ~ mean energy per hit bar in the test beam

run_tb() {   # run_tb <name> <keys...>
	local name=$1; shift
	[ -f calib/$name/py_tb.json ] && return
	rm -rf calib/$name; mkdir -p calib/$name
	printf '%s\n' "$@" | tr '=' ' ' > calib/$name.keys
	[ -f calib/beam.in ] || { (cd calib && cp "$src/TB_eta_F.root" "$src/beam_profile.cpp" . && root -l -b -q "beam_profile.cpp(200000)" > /dev/null); }
	BEAMIN=$out/calib/beam.in bash "$src/scripts/run_scan.sh" "$build" calib/$name tb 200000 calib/$name.keys > /dev/null
	python "$src/analysis/sith_reco.py" tb --runs "calib/$name/output/*MeV_t*-{run}.root" --nruns 7 --json calib/$name/py_tb.json | tail -1
}

echo "== 1. PHOTOSTAT calibration on the test beam (same seeds for all)"
mkdir -p calib
run_tb const_0.22 TRACKERSMEARING=0.22
for s in 0.17 0.175 0.18 0.185 0.19; do
	run_tb phot_$s TRACKERSMEARING=$s TRACKERSMEARMODEL=PHOTOSTAT TRACKERSMEARREF=$EREF
done
SPHOT=$(python - "$out/calib" <<'EOF'
import json, sys, glob, numpy as np
d = sys.argv[1]
target = json.load(open(f"{d}/const_0.22/py_tb.json"))["sigma_mm_mean"]
s, y = [], []
for f in sorted(glob.glob(f"{d}/phot_*/py_tb.json")):
    s.append(float(f.split("phot_")[1].split("/")[0])); y.append(json.load(open(f))["sigma_mm_mean"])
s, y = np.array(s), np.array(y)
a, b = np.polyfit(s, y, 1)
best = (target - b) / a
json.dump(dict(target_mm=target, smear=s.tolist(), sigma_mm=y.tolist(), slope=a, intercept=b,
               photostat_smear=best), open(f"{d}/calibration.json", "w"), indent=1)
print(f"{best:.4f}")
EOF
)
echo "PHOTOSTAT smearing reproducing CONSTANT 0.22 on the test beam: $SPHOT (E_ref = $EREF MeV)"

echo "== 2. overlap check"
mkdir -p geom && printf '/geometry/test/tolerance 0.0001 mm\n/geometry/test/run\n' > geom/test.mac
for b in "${BASES[@]}"; do for h in "${HEIGHTS[@]}"; do
	n=$((420 / b))
	cfg=geom/b${b}_h${h}.cfg
	cp "$build/config.cfg" $cfg
	printf 'NTHREADS 1\nTRACKERCONFIG PERPENDICULAR\nTRACKERSETUP ALL\nTRACKERBASE %s\nTRACKERHEIGHT %s\nTRACKERNBARS %s\nTRACKERLENGTH 420\n' $b $h $n >> $cfg
	(cd geom && mkdir -p output && "$build/jepo" -c b${b}_h${h}.cfg -m test.mac > b${b}_h${h}.log 2>&1) || true
	nov=$(grep -c -E 'Overlap is detected|GeomVol1002' geom/b${b}_h${h}.log || true)
	echo "base $b height $h: $nov overlaps"
	[ "$nov" = 0 ] || { echo "OVERLAPS: stop"; exit 1; }
done; done

echo "== 3. scan"
for model in PHOTOSTAT CONSTANT; do
	for b in "${BASES[@]}"; do for h in "${HEIGHTS[@]}"; do
		n=$((420 / b)); name=${model,,}/b${b}_h${h}
		[ -f $name/py_uniform.json ] && continue
		rm -rf $name; mkdir -p $name
		{ printf 'TRACKERBASE %s\nTRACKERHEIGHT %s\nTRACKERNBARS %s\nTRACKERLENGTH 420\nTRACKERSETUP ALL\n' $b $h $n
		  if [ $model = PHOTOSTAT ]; then printf 'TRACKERSMEARMODEL PHOTOSTAT\nTRACKERSMEARING %s\nTRACKERSMEARREF %s\n' $SPHOT $EREF
		  else printf 'TRACKERSMEARMODEL CONSTANT\nTRACKERSMEARING 0.22\n'; fi; } > $name.keys
		t0=$(date +%s)
		bash "$src/scripts/run_scan.sh" "$build" $name uniform $nev $name.keys > /dev/null
		python "$src/analysis/sith_reco.py" uniform --runs "$name/output/*MeV_t*-{run}.root" \
			--geometry $name/config_last.cfg --json $name/py_uniform.json | tail -1 | sed "s|^|$model b=$b h=$h ($(( $(date +%s) - t0 )) s): |"
	done; done
done

echo "== 4. summary"
python - "$out" <<'EOF'
import json, sys, glob, csv
d = sys.argv[1]
rows = []
for f in sorted(glob.glob(f"{d}/*/b*_h*/py_uniform.json")):
    r = json.load(open(f)); g = r["geometry"]
    model = f.split("/")[-3]
    rows.append(dict(model=model, base_mm=10 * g["Tb"], height_mm=10 * g["Tt"], bars_per_row=g["Tn"],
                     channels=4 * g["Tn"], sigma_theta_deg=r["sigma_theta_mean"], sigma_theta_err=r["sigma_theta_err"],
                     sigma_phi_A_deg=r["sigma_phi_A"], sigma_phi_A_err=r["sigma_phi_A_err"],
                     sigma_r_mm=r["sigma_r_mm"], sigma_r_err=r["sigma_r_err"],
                     sigma_rphi_mm=r["sigma_rphi_mm"], sigma_rphi_err=r["sigma_rphi_err"],
                     efficiency=r["efficiency"], n_events=r["n_events"]))
with open(f"{d}/summary.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)
print(f"{len(rows)} points -> {d}/summary.csv")
EOF
