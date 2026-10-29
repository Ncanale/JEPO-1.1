#!/bin/bash
# Run a JEPO scan with fixed seeds, replacing the source-editing logic of run.sh.
#
#   scripts/run_scan.sh <jepo-build-dir> <out-dir> <theta|tb|uniform> <events-per-run> [keys-file]
#
#   theta    full detector (PERPENDICULAR), one fixed theta per run (5..17.5 deg, 26 runs)
#   tb       2018 test-beam setup (PARALLEL), back layer moved in 7 steps
#   uniform  full detector, one run with theta uniform in [MINTHETA, MAXTHETA]
#
# keys-file: optional "KEY value" lines overriding config.cfg (e.g. the tracker
# geometry). Environment: SEED0 (default 1000, run k uses SEED0+k), NTHR,
# BEAMIN (reuse an existing beam.in for tb), NRUNS (limit number of runs),
# PRECMD (UI commands before beamOn, e.g. '/process/eLoss/setFluctModel Urban').
set -e
build=$(realpath "$1"); out="$2"; mode="$3"; nev="$4"; keys="$5"
src=$(realpath "$(dirname "$0")/..")
SEED0=${SEED0:-1000}
NTHR=${NTHR:-$(nproc)}

mkdir -p "$out/output"
cd "$out"
cp "$build/config.cfg" config.cfg
# PRECMD: optional Geant4 UI commands (';'-separated) executed before beamOn
{ [ -n "$PRECMD" ] && echo "$PRECMD" | tr ';' '\n'
  printf '/run/printProgress %d\n/run/beamOn %d\n' $((nev / 10)) "$nev"; } > n_event.mac

setkey() {   # setkey KEY value...: replace the active line or append it
	local k=$1; shift
	if grep -q "^$k\b" config.cfg; then sed -i "s|^$k\b.*|$k $*|" config.cfg
	else echo "$k $*" >> config.cfg; fi
}
setkey NTHREADS "$NTHR"
setkey PARTICLENAME deuteron
setkey BEAMKINETICENERGY 270
setkey USETARGET OFF
setkey BEAMDELTAX 0.146
setkey BEAMDELTAY 0.195

case $mode in
	theta)
		values=(5 5.5 6 6.5 7 7.5 8 8.5 9 9.5 10 10.5 11 11.5 12 12.5 13 13.5 14 14.5 15 15.5 16 16.5 17 17.5)
		setkey TRACKERCONFIG PERPENDICULAR
		setkey TRACKERSETUP 1111111111111111111111111111
		setkey GENANGLE THETA
		setkey BEAMFILE OFF ;;
	uniform)
		values=(uniform)
		setkey TRACKERCONFIG PERPENDICULAR
		setkey TRACKERSETUP 1111111111111111111111111111
		setkey GENANGLE THETA
		setkey MINTHETA 5
		setkey MAXTHETA 17.5
		setkey BEAMFILE OFF ;;
	tb)
		# nominal 5 mm steps, as corrected by the G4 benchmark (~4.78 mm)
		values=(0.24739106414386924 5.065845196500294 10.152798578987705 15.218870720097698 20.038899210298244 24.438954737516877 28.93078785640231)
		setkey TRACKERCONFIG PARALLEL
		setkey TRACKERSETUP 0001000000110000010000001100
		setkey GENANGLE THETA
		setkey MINTHETA 0.0
		setkey MAXTHETA 0.0
		setkey BEAMFILE ON
		if [ -n "$BEAMIN" ]; then cp "$BEAMIN" beam.in
		else
			cp "$src/TB_eta_F.root" "$src/beam_profile.cpp" .
			root -l -b -q "beam_profile.cpp($nev)" > /dev/null
		fi ;;
	*) echo "unknown mode $mode"; exit 1 ;;
esac

# user overrides last, so they win
if [ -n "$keys" ]; then
	while read -r k v; do [ -n "$k" ] && [ "${k:0:1}" != "#" ] && setkey "$k" "$v"; done < "$keys"
fi

nruns=${NRUNS:-${#values[@]}}
for (( i=0; i<nruns; i++ )); do
	v=${values[$i]}
	case $mode in
		theta) setkey MINTHETA "$v"; setkey MAXTHETA "$v" ;;
		tb)    setkey TRANSLATE "$v" ;;
	esac
	"$build/jepo" -c config.cfg -m n_event.mac -s $((SEED0 + i)) > "run$i.log" 2>&1
	for f in output/*MeV.root output/*MeV_t*.root; do
		[[ $f =~ -[0-9]+\.root$ ]] && continue
		[ -e "$f" ] && mv "$f" "${f%.root}-$i.root"
	done
	echo "run $i ($mode $v) done"
done
cp config.cfg config_last.cfg
