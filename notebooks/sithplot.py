"""Shared plotting style and helpers for the SiTH notebooks.

Palette: fixed-order categorical slots (validated: adjacent CVD dE >= 9.1,
normal-vision >= 19.6 on the light surface). Slots 3-5 are below 3:1 contrast,
so charts using them carry direct labels and a table of the plotted values.
"""
import sys
from pathlib import Path

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

REPO = Path(__file__).resolve().parents[1]
RUNS = Path.home() / "runs"
sys.path.insert(0, str(REPO / "analysis"))

SERIES = ["#2a78d6", "#eb6834", "#1baf7a", "#eda100", "#e87ba4"]   # fixed order
BLUE_RAMP = ["#cde2fb", "#b7d3f6", "#9ec5f4", "#86b6ef", "#6da7ec", "#5598e7",
             "#3987e5", "#2a78d6", "#256abf", "#1c5cab", "#184f95", "#104281", "#0d366b"]
INK, INK2, MUTED = "#0b0b0b", "#52514e", "#898781"
GRID, AXIS, SURFACE = "#e1e0d9", "#c3c2b7", "#fcfcfb"


def setup():
    if "sith_blue" not in mpl.colormaps:
        mpl.colormaps.register(mpl.colors.LinearSegmentedColormap.from_list("sith_blue", BLUE_RAMP))
    mpl.rcParams.update({
        "figure.facecolor": SURFACE, "axes.facecolor": SURFACE, "savefig.facecolor": SURFACE,
        "figure.dpi": 110, "figure.figsize": (7.5, 4.2),
        "font.family": "sans-serif", "font.size": 10,
        "text.color": INK, "axes.labelcolor": INK2, "axes.titlecolor": INK,
        "axes.titlesize": 11, "axes.titleweight": "bold", "axes.titlelocation": "left",
        "axes.edgecolor": AXIS, "axes.linewidth": 0.8,
        "axes.spines.top": False, "axes.spines.right": False,
        "axes.grid": True, "grid.color": GRID, "grid.linewidth": 0.6,
        "axes.axisbelow": True,
        "xtick.color": MUTED, "ytick.color": MUTED, "xtick.labelcolor": INK2, "ytick.labelcolor": INK2,
        "axes.prop_cycle": mpl.cycler(color=SERIES),
        "lines.linewidth": 2, "lines.markersize": 6,
        "legend.frameon": False, "legend.labelcolor": INK2,
        "image.cmap": "sith_blue",
    })


def end_label(ax, x, y, text, color, dx=4, dy=0, **kw):
    """Direct label at the end of a line, in text ink with a colour swatch.
    dx, dy: offset in points (use dy to separate labels of close lines)."""
    ax.annotate("●", (x, y), xytext=(dx, dy), textcoords="offset points",
                color=color, va="center", fontsize=8)
    ax.annotate(text, (x, y), xytext=(dx + 9, dy), textcoords="offset points",
                color=INK2, va="center", fontsize=9, **kw)


def hist_line(ax, values, bins, rng, color, label=None, density=False, **kw):
    """Histogram drawn as a 2 px step outline (no filled bars)."""
    h, e = np.histogram(values[np.isfinite(values)], bins=bins, range=rng, density=density)
    ax.stairs(h, e, color=color, linewidth=1.6, label=label, **kw)
    return h, e
