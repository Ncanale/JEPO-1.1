#!/bin/bash
# Phase 3: resolution vs triangular-bar size at fixed active area (420 mm).
#   1. calibrate the PHOTOSTAT and NOISE smearing on the test-beam setup, so that
#      they give the same offset distributions as the CONSTANT 0.22 tuning to the
#      2018 data
#   2. Geant4 overlap check of every geometry
#   3. one 'uniform' run per geometry and smearing model, analysed with sith_reco.py
#      Models: CONSTANT 0.22, PHOTOSTAT (same light yield for every bar), NOISE
#      (sigma_E independent of E, calibrated like PHOTOSTAT) and,
#      when OPTICAL_CSV points to the optical/sithopt summary
#      (analysis/optical_summary.py), OPTICAL: PHOTOSTAT scaled with the
#      simulated light-collection efficiency, s = s_proto * sqrt(eps_proto / eps)
#
#   [OPTICAL_CSV=summary.csv] scripts/scan_bars.sh <out-dir> [events-per-point]
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
for s in 0.11 0.13 0.15 0.17 0.19; do
	run_tb noise_$s TRACKERSMEARING=$s TRACKERSMEARMODEL=NOISE TRACKERSMEARREF=$EREF
done
read -r SPHOT SNOISE < <(python - "$out/calib" <<'EOF'
import json, sys, glob, numpy as np
d = sys.argv[1]
target = json.load(open(f"{d}/const_0.22/py_tb.json"))["sigma_mm_mean"]
def points(prefix):
    s, y = [], []
    for f in sorted(glob.glob(f"{d}/{prefix}_*/py_tb.json")):
        s.append(float(f.split(f"{prefix}_")[1].split("/")[0])); y.append(json.load(open(f))["sigma_mm_mean"])
    return np.array(s), np.array(y)
# PHOTOSTAT: linear fit on a narrow grid
s, y = points("phot")
a, b = np.polyfit(s, y, 1)
best = (target - b) / a
# NOISE: wide grid, sigma^2 = c0 + c1 s^2 (smearing added in quadrature)
sn, yn = points("noise")
c1, c0 = np.polyfit(sn ** 2, yn ** 2, 1)
best_n = np.sqrt((target ** 2 - c0) / c1)
json.dump(dict(target_mm=target, smear=s.tolist(), sigma_mm=y.tolist(), slope=a, intercept=b,
               photostat_smear=best, noise_smear_values=sn.tolist(), noise_sigma_mm=yn.tolist(),
               noise_c0=c0, noise_c1=c1, noise_smear=best_n), open(f"{d}/calibration.json", "w"), indent=1)
print(f"{best:.4f} {best_n:.4f}")
EOF
)
echo "PHOTOSTAT smearing reproducing CONSTANT 0.22 on the test beam: $SPHOT (E_ref = $EREF MeV)"
echo "NOISE smearing reproducing CONSTANT 0.22 on the test beam: $SNOISE (E_ref = $EREF MeV)"

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

optical_smear() {   # optical_smear <base> <height>: PHOTOSTAT smearing scaled with 1/sqrt(eps)
	python - "$OPTICAL_CSV" $1 $2 $SPHOT <<'EOF'
import csv, sys
f, b, h, s = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), float(sys.argv[4])
for r in csv.DictReader(open(f)):
    if r["variant"] == "nominal" and int(r["base_mm"]) == b and int(r["height_mm"]) == h:
        print(f"{s / float(r['eps_rel']) ** 0.5:.5f}"); break
else:
    sys.exit(f"no optical run for base {b} height {h}")
EOF
}

echo "== 3. scan"
MODELS=(PHOTOSTAT CONSTANT NOISE)
[ -n "$OPTICAL_CSV" ] && OPTICAL_CSV=$(realpath "$OPTICAL_CSV") && MODELS+=(OPTICAL)
for model in "${MODELS[@]}"; do
	for b in "${BASES[@]}"; do for h in "${HEIGHTS[@]}"; do
		n=$((420 / b)); name=${model,,}/b${b}_h${h}
		[ -f $name/py_uniform.json ] && continue
		rm -rf $name; mkdir -p $name
		{ printf 'TRACKERBASE %s\nTRACKERHEIGHT %s\nTRACKERNBARS %s\nTRACKERLENGTH 420\nTRACKERSETUP ALL\n' $b $h $n
		  if [ $model = PHOTOSTAT ]; then printf 'TRACKERSMEARMODEL PHOTOSTAT\nTRACKERSMEARING %s\nTRACKERSMEARREF %s\n' $SPHOT $EREF
		  elif [ $model = NOISE ]; then printf 'TRACKERSMEARMODEL NOISE\nTRACKERSMEARING %s\nTRACKERSMEARREF %s\n' $SNOISE $EREF
		  elif [ $model = OPTICAL ]; then printf 'TRACKERSMEARMODEL PHOTOSTAT\nTRACKERSMEARING %s\nTRACKERSMEARREF %s\n' $(optical_smear $b $h) $EREF
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
