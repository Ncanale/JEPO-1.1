"""Figures of the bar-size and light-collection study for the SiTH paper.

Reads ~/runs/scan_bars/summary.csv and the optical/sithopt runs in
~/runs/optical, writes PDF figures to the output directory.

  python paper_figures.py <out-dir>
"""
import sys
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "notebooks"))
import sithplot as sp          # noqa: E402
import optical_summary as osum  # noqa: E402

MODELS = ["CONSTANT", "PHOTOSTAT", "NOISE", "OPTICAL"]
KEY = [(60, 20), (42, 30), (30, 30), (20, 20), (20, 10)]


def scan_vs_base(df, out, model="CONSTANT"):
    """sigma_r and sigma_theta vs base, one line per height; band: spread of
    the smearing models at h = 20 mm."""
    d = df[df.model == model]
    band = df[df.height_mm == 20].groupby("base_mm")
    fig, axs = plt.subplots(1, 2, figsize=(7.2, 3.0))
    for ax, (col, lab) in zip(axs, (("sigma_r_mm", r"$\sigma_r$ [mm]"), ("sigma_theta_deg", r"$\sigma_\theta$ [deg]"))):
        lo, hi = band[col].min(), band[col].max()
        ax.fill_between(lo.index, lo.values, hi.values, color=sp.SERIES[3], alpha=0.25, lw=0,
                        label="h = 20 mm, all models")
        for i, h in enumerate(sorted(d.height_mm.unique())):
            g = d[d.height_mm == h].sort_values("base_mm")
            ax.plot(g.base_mm, g[col], "o-", color=sp.SERIES[i], ms=4, lw=1.6, label=f"h = {h:.0f} mm")
        p = d[(d.base_mm == 60) & (d.height_mm == 20)].iloc[0]
        ax.plot(60, p[col], "o", ms=10, mfc="none", mec=sp.INK, mew=1.0)
        ax.set(xlabel="triangle base $b$ [mm]", ylabel=lab, ylim=(0, None), title="")
    axs[0].legend(fontsize=7, loc="upper left")
    fig.tight_layout()
    fig.savefig(out / "bar_scan_vs_base.pdf")
    plt.close(fig)


def scan_models(df, out):
    """sigma_r vs height for the smearing models, one panel per base."""
    bases = [20, 30, 42, 60]
    fig, axs = plt.subplots(1, len(bases), figsize=(7.2, 2.6))
    for ax, b in zip(axs, bases):
        for i, m in enumerate(MODELS):
            g = df[(df.model == m) & (df.base_mm == b)].sort_values("height_mm")
            ax.plot(g.height_mm, g.sigma_r_mm, "o-", color=sp.SERIES[i], ms=3.5, lw=1.4, label=m)
        top = 1.8 * df[(df.base_mm == b) & (df.height_mm == 20)].sigma_r_mm.max()
        ax.set(title=f"$b$ = {b} mm", xlabel="height $h$ [mm]", ylim=(0, top))
        ax.title.set_fontsize(9)
    axs[0].set_ylabel(r"$\sigma_r$ [mm]")
    handles, labels = axs[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="lower center", ncol=len(MODELS), fontsize=7)
    fig.tight_layout(rect=(0, 0.08, 1, 1))
    fig.savefig(out / "bar_scan_models.pdf")
    plt.close(fig)


def light_collection(runs, out):
    """Collection efficiency relative to the prototype, and profile along the bar."""
    opt = pd.DataFrame(osum.summary(runs))
    nom = opt[opt.variant == "nominal"]
    bases, heights = sorted(nom.base_mm.unique()), sorted(nom.height_mm.unique())
    M = nom.pivot(index="height_mm", columns="base_mm", values="eps_rel").loc[heights, bases]
    fig, axs = plt.subplots(1, 2, figsize=(7.2, 3.0), gridspec_kw=dict(width_ratios=[1, 1.15]))
    ax = axs[0]
    ax.imshow(M.values, origin="lower", cmap="sith_blue", aspect="auto")
    lo, hi = np.nanmin(M.values), np.nanmax(M.values)
    for (yi, xi), v in np.ndenumerate(M.values):
        ax.text(xi, yi, f"{v:.2f}", ha="center", va="center", fontsize=7,
                color="white" if (v - lo) / (hi - lo) > 0.55 else sp.INK)
    ax.set(xticks=range(len(bases)), xticklabels=bases, yticks=range(len(heights)), yticklabels=heights,
           xlabel="base $b$ [mm]", ylabel="height $h$ [mm]")
    ax.grid(False)
    i, j = heights.index(20), bases.index(60)
    ax.add_patch(plt.Rectangle((j - 0.5, i - 0.5), 1, 1, fill=False, ec=sp.INK, lw=1.2))
    ax = axs[1]
    for k, (b, h) in enumerate(KEY):
        m = osum.metrics(osum.load_run(Path(runs) / f"b{b}_h{h}.root"))
        ax.plot(m["z"], m["eps_z"] / m["eps"], color=sp.SERIES[k], lw=1.4, label=f"{b} x {h} mm")
    ax.set(xlabel="distance from the read-out end [mm]", ylabel=r"$\varepsilon(z)/\bar\varepsilon$")
    ax.legend(fontsize=6.5, loc="upper right")
    fig.tight_layout()
    fig.savefig(out / "light_collection.pdf")
    plt.close(fig)


def main():
    out = Path(sys.argv[1]); out.mkdir(parents=True, exist_ok=True)
    sp.setup()
    plt.rcParams.update({"figure.dpi": 150, "font.size": 8, "axes.titlesize": 9, "axes.labelsize": 8,
                         "xtick.labelsize": 7, "ytick.labelsize": 7, "savefig.facecolor": "white",
                         "figure.facecolor": "white", "axes.facecolor": "white"})
    df = pd.read_csv(sp.RUNS / "scan_bars" / "summary.csv")
    df["model"] = df.model.str.upper()
    scan_vs_base(df, out)
    scan_models(df, out)
    light_collection(sp.RUNS / "optical", out)
    print("written:", ", ".join(p.name for p in sorted(out.glob("*.pdf"))))


if __name__ == "__main__":
    main()
