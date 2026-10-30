"""Light collection of the SiTH triangular bars from optical/sithopt runs.

For every run: collection efficiency eps (photons reaching the SiPMs over
photons emitted, emission uniform in the bar volume, i.e. bars uniformly
illuminated by tracks crossing the triangle), its profile along the bar and
the response of a track crossing the triangle at a given transverse position.

Normalisation to the test beam (eps of the 60 x 20 mm prototype): if all the
test-beam smearing is photostatistics (PHOTOSTAT, s = S_PROTO at E_REF), a bar
with efficiency eps has s = S_PROTO * sqrt(eps_proto / eps).

Usage
  python optical_summary.py ~/runs/optical --csv ~/runs/optical/summary.csv
"""
import argparse
import csv
import re
from pathlib import Path

import numpy as np
import uproot

S_PROTO = 0.1799      # PHOTOSTAT smearing calibrated on the 2018 test beam
E_REF = 7.0           # MeV
PROTO = (60, 20)      # mm, base and height of the prototype bars

# Absolute scale, only for the comparison with the test-beam bound
LIGHT_YIELD = 10000.0  # photons / MeV, EJ-200 data sheet
BIRKS = 0.93           # quenching for ~5.5 MeV/cm (270 MeV deuterons), kB = 0.126 mm/MeV
PDE_EFF = 0.35         # assumed effective PDE of the SiPMs over the EJ-200 spectrum

TAG = re.compile(r"b(\d+)_h(\d+)(?:_(.+))?$")


def load_run(root):
    """Histograms of one run: generated and detected photons."""
    root = Path(root)
    log = root.with_suffix(".log").read_text()
    m = TAG.match(root.stem)
    base, height, variant = int(m.group(1)), int(m.group(2)), m.group(3) or "nominal"
    area = sum(float(a) for a in re.findall(r"sensitive area ([\d.]+) mm2", log))
    run = dict(tag=root.stem, base_mm=base, height_mm=height, variant=variant,
               layout="BOTH" if variant == "both" else "ONE",
               sensor_area_mm2=area, face_area_mm2=0.5 * base * height)
    with uproot.open(root) as f:
        for name in ("gen_z", "det_z", "gen_lambda", "det_lambda", "det_time"):
            run[name], run[name + "_edges"] = f[name].to_numpy()
        for name in ("gen_uv", "det_uv", "gen_uz", "det_uz"):
            h, ex, ey = f[name].to_numpy()
            run[name], run[name + "_xedges"], run[name + "_yedges"] = h, ex, ey
    return run


def ratio(det, gen):
    with np.errstate(divide="ignore", invalid="ignore"):
        return np.where(gen > 0, det / gen, np.nan)


def metrics(run):
    """Efficiency, profile along the bar and transverse response."""
    n_gen, n_det = run["gen_z"].sum(), run["det_z"].sum()
    eps_z = ratio(run["det_z"], run["gen_z"])
    z = 0.5 * (run["gen_z_edges"][1:] + run["gen_z_edges"][:-1])
    L = run["gen_z_edges"][-1]
    near, far, mid = z < 0.1 * L, z > 0.9 * L, np.abs(z - L / 2) < 0.05 * L
    # response of a track crossing the triangle at u = 2x/base: emission
    # uniform in the cross-section, so the column average is the mean
    # efficiency along the chord
    eps_u = ratio(run["det_uv"].sum(axis=1), run["gen_uv"].sum(axis=1))
    u = 0.5 * (run["gen_uv_xedges"][1:] + run["gen_uv_xedges"][:-1])
    core = np.abs(u) < 0.9
    return dict(eps=n_det / n_gen, eps_err=np.sqrt(n_det) / n_gen, n_gen=n_gen,
                eps_mid=run["det_z"][mid].sum() / run["gen_z"][mid].sum(),
                eps_near_far=(run["det_z"][near].sum() / run["gen_z"][near].sum())
                / (run["det_z"][far].sum() / run["gen_z"][far].sum()),
                u_rms=np.nanstd(eps_u[core]) / np.nanmean(eps_u[core]),
                eps_z=eps_z, z=z, eps_u=eps_u, u=u)


def npe_per_mev(eps):
    """Expected photoelectrons per MeV from the absolute optical model."""
    return LIGHT_YIELD * BIRKS * PDE_EFF * eps


def summary(runs_dir):
    rows = []
    for root in sorted(Path(runs_dir).glob("b*_h*.root")):
        log = root.with_suffix(".log")
        if not TAG.match(root.stem) or not log.exists() or "collection efficiency" not in log.read_text():
            continue   # not a sithopt run, or still running
        run = load_run(root)
        m = metrics(run)
        rows.append({k: run[k] for k in ("tag", "base_mm", "height_mm", "variant", "layout",
                                         "sensor_area_mm2", "face_area_mm2")}
                    | {k: m[k] for k in ("eps", "eps_err", "eps_mid", "eps_near_far", "u_rms", "n_gen")})
    # relative to the prototype with the same optics
    proto = {r["variant"]: r["eps"] for r in rows if (r["base_mm"], r["height_mm"]) == PROTO}
    for r in rows:
        r["eps_rel"] = r["eps"] / proto[r["variant"]] if r["variant"] in proto else np.nan
        r["smear_photostat"] = S_PROTO / np.sqrt(r["eps_rel"])
        r["npe_per_mev_abs"] = npe_per_mev(r["eps"])
    return rows


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("runs_dir")
    ap.add_argument("--csv", help="write the summary table here")
    args = ap.parse_args()
    rows = summary(args.runs_dir)
    for r in rows:
        print(f"{r['tag']:22s} area {r['sensor_area_mm2']:6.1f} mm2  eps {r['eps']:.4f} +- {r['eps_err']:.4f}  "
              f"rel {r['eps_rel']:.3f}  s {r['smear_photostat']:.4f}  near/far {r['eps_near_far']:.2f}  "
              f"u_rms {r['u_rms']:.3f}  {r['npe_per_mev_abs']:.0f} pe/MeV")
    if args.csv:
        with open(args.csv, "w", newline="") as fh:
            w = csv.DictWriter(fh, fieldnames=list(rows[0]))
            w.writeheader()
            w.writerows(rows)
        print(f"{len(rows)} runs -> {args.csv}")


if __name__ == "__main__":
    main()
