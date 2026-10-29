"""SiTH reconstruction and resolution analysis on JEPO ntuples (uproot + numpy).

Python port of output/Simulation_runner.cpp + output/Peak_fitter.py with the
tracker geometry passed as parameters instead of being hard-coded, so that
different triangular-bar sizes can be analysed with the same code.

With the default Geometry() the reconstruction is the same as the original
ROOT macro (same pair selection, same formulas).

Usage examples
  python sith_reco.py theta  --runs 'output/deuteronEmpty-270MeV_t*-{run}.root' --nruns 26
  python sith_reco.py tb     --runs 'output/deuteronEmpty-270MeV_t*-{run}.root' --nruns 7
  python sith_reco.py theta  --geometry output/geometry.json ...
"""
import argparse
import glob
import json
from dataclasses import dataclass, asdict

import numpy as np
import uproot
from scipy.optimize import curve_fit

DEG = 180.0 / np.pi


@dataclass
class Geometry:
    """Tracker geometry in cm (as in Simulation_runner.cpp)."""
    Tb: float = 6.0       # module base
    Tt: float = 2.0       # module thickness (triangle height)
    Tn: int = 7           # modules per row (front-facing modules)
    dF: float = 59.0      # source -> start of front layer
    dB: float = 61.0      # source -> start of back layer

    @property
    def CN(self):
        return 2 * self.Tn

    @classmethod
    def from_json(cls, path):
        with open(path) as f:
            d = json.load(f)
        return cls(**{k: d[k] for k in asdict(cls()) if k in d})

    @classmethod
    def from_config(cls, path):
        """Geometry of a jepo config.cfg (lengths there are in mm).

        The layers sit 30 mm in front of the LYSO face (DETECTORPOSITIONZ),
        front layer first: dF = Z - 30 - 2h, dB = Z - 30 - h (59, 61 cm by default).
        """
        keys = dict(TRACKERBASE=60.0, TRACKERHEIGHT=20.0, TRACKERNBARS=7,
                    DETECTORPOSITIONZ=660.0)
        with open(path) as f:
            for line in f:
                w = line.split()
                if len(w) >= 2 and w[0] in keys:
                    keys[w[0]] = float(w[1])
        b, h, z = keys["TRACKERBASE"], keys["TRACKERHEIGHT"], keys["DETECTORPOSITIONZ"]
        return cls(Tb=b / 10, Tt=h / 10, Tn=int(keys["TRACKERNBARS"]),
                   dF=(z - 30 - 2 * h) / 10, dB=(z - 30 - h) / 10)


# --------------------------------------------------------------------------- #
#   Input
# --------------------------------------------------------------------------- #
def load(files, geom, extra=("theta_lab", "phi_lab")):
    """Return F, B (n_events x CN) energy arrays and generator columns."""
    names_F = [f"kE_F_{i + 1:02d}" for i in range(geom.CN)]
    names_B = [f"kE_B_{i + 1:02d}" for i in range(geom.CN)]
    cols = names_F + names_B + list(extra)
    parts = []
    for fn in files:
        with uproot.open(fn) as f:
            if "hits" not in f or f["hits"].num_entries == 0:
                continue
            parts.append(f["hits"].arrays(cols, library="np"))
    if not parts:
        raise RuntimeError(f"no entries in {files}")
    data = {c: np.concatenate([p[c] for p in parts]) for c in cols}
    F = np.stack([data[c] for c in names_F], axis=1)
    B = np.stack([data[c] for c in names_B], axis=1)
    return F, B, {c: data[c] for c in extra}


# --------------------------------------------------------------------------- #
#   Reconstruction
# --------------------------------------------------------------------------- #
def select_pairs(E, Tn, adjacent_only=False):
    """Pick, per event, the (q, r) pair of modules used for eta.

    Like the original loop, the last valid pair in (q, r) order wins.
    Returns q, r (int arrays, -1 where no valid pair) and eta.
    """
    n, CN = E.shape
    ok = ~np.isnan(E)
    q_sel = np.full(n, -1)
    r_sel = np.full(n, -1)
    todo = np.ones(n, bool)
    for q in range(Tn - 1, -1, -1):
        for r in range(CN - 1, Tn - 1, -1):
            if adjacent_only and (r - q - Tn) not in (0, 1):
                continue
            m = todo & ok[:, q] & ok[:, r]
            q_sel[m] = q
            r_sel[m] = r
            todo &= ~m
    found = q_sel >= 0
    eta = np.full(n, np.nan)
    a = E[found, q_sel[found]]
    b = E[found, r_sel[found]]
    eta[found] = (a - b) / (a + b)
    return q_sel, r_sel, eta


def layer_tan(q, r, eta, d, geom):
    """tan of the projected angle measured by one layer (Simulation_runner)."""
    ep = r - q - geom.Tn
    sgn = np.where(ep % 2 == 0, 1.0, -1.0)          # (-1)^ep
    transverse = (geom.Tb / 4) * (2 * geom.Tn - 4 * q - 3 + (1 - eta) * sgn)
    depth = (geom.Tt / 2) * (1 + eta)               # original: "1 + eta" with Tt = 2 cm
    t = transverse / (d + depth)
    return np.where(q >= 0, t, np.nan)


def reconstruct(F, B, geom, adjacent_only=False):
    """Reconstructed theta, phi (deg) at the front-layer distance."""
    qF, rF, etaF = select_pairs(F, geom.Tn, adjacent_only)
    qB, rB, etaB = select_pairs(B, geom.Tn, adjacent_only)
    yl = geom.dF * layer_tan(qF, rF, etaF, geom.dF, geom)
    xl = geom.dF * layer_tan(qB, rB, etaB, geom.dB, geom)
    theta = DEG * np.arctan(np.hypot(xl, yl) / geom.dF)
    phi = DEG * np.arctan2(yl, xl)
    return dict(theta=theta, phi=phi, xl=xl, yl=yl,
                qF=qF, rF=rF, etaF=etaF, qB=qB, rB=rB, etaB=etaB)


# --------------------------------------------------------------------------- #
#   Fitting
# --------------------------------------------------------------------------- #
def gaus(x, a, mu, s):
    return a * np.exp(-0.5 * ((x - mu) / s) ** 2)


def multigaus(x, *p):
    return sum(gaus(x, *p[3 * j:3 * j + 3]) for j in range(len(p) // 3))


def fit_hist(values, bins, rng, p0, fit_range=None):
    """Chi2 fit of a (multi-)Gaussian to a histogram, Poisson errors."""
    h, edges = np.histogram(values[np.isfinite(values)], bins=bins, range=rng)
    x = 0.5 * (edges[1:] + edges[:-1])
    m = h > 0
    if fit_range is not None:
        m &= (x >= fit_range[0]) & (x <= fit_range[1])
    p, cov = curve_fit(multigaus, x[m], h[m], p0=p0, sigma=np.sqrt(h[m]),
                       absolute_sigma=True, maxfev=200000)
    p = np.array(p)
    err = np.sqrt(np.diag(cov))
    chi2 = np.sum(((h[m] - multigaus(x[m], *p)) / np.sqrt(h[m])) ** 2)
    return p, err, chi2, m.sum() - len(p)


def fit_peak(values, center, sigma0, half_width=None, bins=None, rng=None):
    """Single-Gaussian fit around one known peak (iterated +-2.5 sigma window)."""
    if bins is None:
        bins, rng = 400, (0, 20)        # HTh binning of Simulation_runner
    lo, hi = (center - (half_width or 5 * sigma0), center + (half_width or 5 * sigma0))
    s = sigma0
    for _ in range(3):
        p, e, chi2, ndf = fit_hist(values, bins, rng, [values.size / 10, center, s],
                                   fit_range=(lo, hi))
        center, s = p[1], abs(p[2])
        lo, hi = center - 2.5 * s, center + 2.5 * s
    return p, e, chi2, ndf


# --------------------------------------------------------------------------- #
#   Analyses
# --------------------------------------------------------------------------- #
THETA_ARRAY = [5 + 0.5 * i for i in range(26)]
TB_TRANSLATE = [0.24739106414386924, 5.065845196500294, 10.152798578987705,
                15.218870720097698, 20.038899210298244, 24.438954737516877,
                28.93078785640231]
TB_PEAKS = [-1.39, -1.06, -0.73, -0.4, -0.06, 0.23, 0.53]
TB_YMAX = [334, 1100, 1550, 1625, 1458, 1050, 468]


def run_files(pattern, run):
    files = sorted(glob.glob(pattern.format(run=run)))
    if not files:
        raise FileNotFoundError(pattern.format(run=run))
    return files


def analyse_theta(pattern, nruns, geom, thetas=THETA_ARRAY, adjacent_only=False,
                  multi=True):
    """theta scan: one fixed generated theta per run (run.sh, angle=THETA)."""
    all_theta, per_run = [], []
    for k in range(nruns):
        F, B, gen = load(run_files(pattern, k), geom)
        rec = reconstruct(F, B, geom, adjacent_only)
        th = rec["theta"]
        all_theta.append(th)
        p, e, chi2, ndf = fit_peak(th, thetas[k], 0.1, half_width=0.5)
        per_run.append(dict(theta_gen=thetas[k], mean=p[1], sigma=abs(p[2]),
                            sigma_err=e[2], n=int(np.isfinite(th).sum()),
                            chi2ndf=chi2 / max(ndf, 1)))
    out = dict(per_run=per_run,
               sigma_mean=float(np.mean([r["sigma"] for r in per_run])),
               sigma_err_mean=float(np.mean([r["sigma_err"] for r in per_run])))
    if multi:
        # same procedure as Peak_fitter.py: n-Gaussian fit of the summed histogram
        th = np.concatenate(all_theta)
        hmax = np.histogram(th[np.isfinite(th)], 400, (0, 20))[0].max()
        p0 = sum(([hmax * 0.99, t, 0.05] for t in thetas[:nruns]), [])
        p, e, chi2, ndf = fit_hist(th, 400, (0, 20), p0,
                                   fit_range=(min(thetas[:nruns]) - 2, max(thetas[:nruns]) + 2))
        sig = np.abs(p[2::3])
        out.update(multi_sigma_mean=float(sig.mean()),
                   multi_sigma_err_mean=float(e[2::3].mean()),
                   multi_chi2ndf=float(chi2 / ndf))
    return out


def analyse_tb(pattern, nruns, geom, pair=(3, 11), translate=TB_TRANSLATE):
    """Test-beam-like PARALLEL analysis: offset = etaF - etaB for one F pair."""
    offsets = []
    for k in range(nruns):
        F, B, _ = load(run_files(pattern, k), geom, extra=())
        qF, rF, etaF = select_pairs(F, geom.Tn)
        qB, rB, etaB = select_pairs(B, geom.Tn)
        m = (qF == pair[0]) & (rF == pair[1]) & (qB >= 0)
        offsets.append(etaF[m] - etaB[m])
    allo = np.concatenate(offsets)
    # Same n-Gaussian fit of the summed histogram as Peak_fitter.py, but started
    # from each step's own peak (Peak_fitter's fixed TB amplitudes only suit
    # ~1e5 events per step: with more statistics the fit can lose some peaks)
    width = 4 / 200
    p0, single = [], []
    for o in offsets:
        med = np.median(o)
        s = 1.4826 * np.median(np.abs(o - med))
        p, _, _, _ = fit_peak(o, med, s, half_width=4 * s, bins=200, rng=(-2, 2))
        single.append(abs(p[2]))
        p0 += [o.size * width / (np.sqrt(2 * np.pi) * abs(p[2])), p[1], abs(p[2])]
    p, e, chi2, ndf = fit_hist(allo, 200, (-2, 2), p0, fit_range=(-1.55, 0.7))
    means, sig, sig_e = p[1::3], np.abs(p[2::3]), e[2::3]
    calib = (translate[nruns - 1] - translate[0]) / (means[-1] - means[0])  # mm / offset
    if np.max(np.abs(means - np.array(p0[1::3]))) > 0.05:
        print("WARNING: n-Gaussian fit moved a peak by > 0.05 from its step's own fit")
    return dict(calib_mm_per_offset=float(calib), means=means.tolist(),
                amplitudes=p[0::3].tolist(),    # per 0.02 bin of the summed histogram
                sigma_mm=(sig * calib).tolist(),
                sigma_mm_mean=float(np.mean(sig * calib)),
                sigma_mm_err_mean=float(np.mean(sig_e * calib)),
                sigma_mm_single=(np.array(single) * calib).tolist(),
                chi2ndf=float(chi2 / ndf))


def analyse_uniform(pattern, geom, nbins=10, adjacent_only=False, run=0):
    """Event-by-event residuals for a run with uniform theta (run_scan.sh uniform).

    In each theta bin the residual distribution is fitted with a Gaussian
    (width around its own mean, i.e. like the fixed-theta scan). Returns
    sigma_theta(theta), sigma_phi(theta), the A of sigma_phi = A cot(theta),
    the effective transverse resolution at the front layer and the fraction
    of events with a reconstructed track.
    """
    F, B, gen = load(run_files(pattern, run), geom)
    rec = reconstruct(F, B, geom, adjacent_only)
    th_gen = DEG * gen["theta_lab"]
    ph_gen = DEG * gen["phi_lab"]
    dth = rec["theta"] - th_gen
    dph = (rec["phi"] - ph_gen + 180) % 360 - 180
    good = np.isfinite(dth)
    edges = np.linspace(th_gen.min(), th_gen.max(), nbins + 1)
    rows = []
    for lo, hi in zip(edges[:-1], edges[1:]):
        m = good & (th_gen >= lo) & (th_gen < hi)
        row = dict(theta=0.5 * (lo + hi), n=int(m.sum()),
                   eff=float(m.sum() / max(((th_gen >= lo) & (th_gen < hi)).sum(), 1)))
        for name, res, s0 in (("theta", dth[m], 0.1), ("phi", dph[m], 1.0)):
            mad = 1.4826 * np.median(np.abs(res - np.median(res)))   # robust start
            s0 = mad if mad > 0 else s0
            p, e, _, _ = fit_peak(res, np.median(res), s0, half_width=5 * s0,
                                  bins=200, rng=(np.median(res) - 6 * s0, np.median(res) + 6 * s0))
            row[f"sigma_{name}"], row[f"sigma_{name}_err"] = abs(p[2]), e[2]
            row[f"bias_{name}"] = p[1]
        rows.append(row)
    th = np.radians([r["theta"] for r in rows])
    sph = np.array([r["sigma_phi"] for r in rows])
    sph_e = np.array([r["sigma_phi_err"] for r in rows])
    cot = 1 / np.tan(th)
    w = 1 / sph_e ** 2
    A = float(np.sum(w * sph * cot) / np.sum(w * cot ** 2))   # weighted LSQ, sigma_phi = A cot
    A_err = float(1 / np.sqrt(np.sum(w * cot ** 2)))
    sth = np.array([r["sigma_theta"] for r in rows])
    return dict(per_bin=rows, sigma_theta_mean=float(sth.mean()),
                sigma_theta_err=float(np.mean([r["sigma_theta_err"] for r in rows])),
                sigma_phi_A=A, sigma_phi_A_err=A_err,
                # transverse resolution at the front layer, mm
                sigma_pos_mm=float(10 * geom.dF * np.radians(sth.mean())),
                efficiency=float(good.mean()), n_events=int(good.size))


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("mode", choices=["theta", "tb", "uniform"])
    ap.add_argument("--runs", required=True,
                    help="glob pattern of per-thread files with {run} placeholder")
    ap.add_argument("--nruns", type=int, default=1)
    ap.add_argument("--geometry", help="geometry from a .json or a jepo config (.cfg) file")
    ap.add_argument("--adjacent-only", action="store_true",
                    help="use only adjacent F/B module pairs for eta")
    ap.add_argument("--json", help="write the result to this file")
    a = ap.parse_args()

    if not a.geometry:
        geom = Geometry()
    elif a.geometry.endswith(".json"):
        geom = Geometry.from_json(a.geometry)
    else:
        geom = Geometry.from_config(a.geometry)
    if a.mode == "theta":
        res = analyse_theta(a.runs, a.nruns, geom, adjacent_only=a.adjacent_only)
        for r in res["per_run"]:
            print(f"theta {r['theta_gen']:5.2f}: mean {r['mean']:.4f}  "
                  f"sigma {r['sigma']:.4f} +- {r['sigma_err']:.4f} deg  (n={r['n']})")
        print(f"AVERAGE sigma (per-run fits): {res['sigma_mean']:.4f} +- "
              f"{res['sigma_err_mean']:.4f} deg")
        print(f"AVERAGE sigma (n-Gauss fit) : {res['multi_sigma_mean']:.4f} +- "
              f"{res['multi_sigma_err_mean']:.4f} deg  chi2/ndf {res['multi_chi2ndf']:.2f}")
    elif a.mode == "uniform":
        res = analyse_uniform(a.runs, geom, adjacent_only=a.adjacent_only)
        for r in res["per_bin"]:
            print(f"theta {r['theta']:5.2f}: sigma_theta {r['sigma_theta']:.4f}  "
                  f"sigma_phi {r['sigma_phi']:.3f} deg  eff {r['eff']:.3f}")
        print(f"sigma_theta {res['sigma_theta_mean']:.4f} +- {res['sigma_theta_err']:.4f} deg, "
              f"sigma_phi = ({res['sigma_phi_A']:.4f} +- {res['sigma_phi_A_err']:.4f}) cot(theta) deg, "
              f"sigma_pos {res['sigma_pos_mm']:.3f} mm, efficiency {res['efficiency']:.3f}")
    else:
        res = analyse_tb(a.runs, a.nruns, geom)
        print(f"calibration {res['calib_mm_per_offset']:.3f} mm/offset")
        print("sigma per step [mm]:", np.round(res["sigma_mm"], 3))
        print(f"AVERAGE sigma: {res['sigma_mm_mean']:.3f} +- {res['sigma_mm_err_mean']:.3f} mm")
    res["geometry"] = asdict(geom)
    if a.json:
        with open(a.json, "w") as f:
            json.dump(res, f, indent=1)


if __name__ == "__main__":
    main()
