"""
Report generation: latest results table + history trend plots.

Outputs:
  testing/reports/latest.png   — per-config accuracy comparison
  testing/reports/history.png  — accuracy trend over commits
"""

import os
import sys
import csv
import math

REPORTS_DIR = os.path.join(os.path.dirname(__file__), "reports")

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    import matplotlib.gridspec as gridspec
    HAS_MPL = True
except ImportError:
    HAS_MPL = False

plt_params = {
    "font.family":    "serif",
    "font.size":      9,
    "axes.titlesize": 10,
    "axes.labelsize": 9,
    "legend.fontsize": 8,
    "axes.grid":      True,
    "grid.alpha":     0.3,
    "grid.linestyle": "--",
    "lines.linewidth": 1.2,
    "figure.dpi":     150,
}

_COLORS = ["#2166ac", "#d6604d", "#4dac26", "#8e44ad",
           "#e67e22", "#16a085", "#c0392b", "#2980b9",
           "#f39c12", "#1abc9c"]


# ── Latest results report ─────────────────────────────────────────────────────

def report_latest(results: list, output_path: str = None):
    """
    results: list of dicts with keys: station, config, metrics, passed, region
    """
    if not HAS_MPL:
        print("matplotlib not available — skipping report")
        return

    if not results:
        return

    if output_path is None:
        os.makedirs(REPORTS_DIR, exist_ok=True)
        output_path = os.path.join(REPORTS_DIR, "latest.png")

    plt.rcParams.update(plt_params)

    # Group by config
    configs = list(dict.fromkeys(r["config"] for r in results))
    stations = list(dict.fromkeys(r["station"] for r in results))

    fig, axes = plt.subplots(1, 3, figsize=(14, 5))
    fig.suptitle("SPP/PPP Positioning Accuracy — Latest Results", fontsize=11, y=1.01)

    metrics_to_plot = [
        ("3d_rms",  "3D RMS (m)"),
        ("3d_p95",  "3D P95 (m)"),
        ("uere",    "UERE (m)"),
    ]

    x = range(len(configs))
    width = 0.8 / max(len(stations), 1)

    for ax, (metric_key, ylabel) in zip(axes, metrics_to_plot):
        for si, station in enumerate(stations):
            vals = []
            colors = []
            for cfg in configs:
                r = next((r for r in results if r["station"] == station and r["config"] == cfg), None)
                v = r["metrics"].get(metric_key, float("nan")) if r else float("nan")
                vals.append(v if not math.isnan(v) else 0)
                colors.append("#2ecc71" if (r and r["passed"]) else "#e74c3c" if r else "#95a5a6")

            offset = (si - len(stations) / 2 + 0.5) * width
            bars = ax.bar([xi + offset for xi in x], vals, width=width * 0.9,
                          label=station, color=_COLORS[si % len(_COLORS)], alpha=0.85)

            # Mark failed bars with hatching
            for bar, r, cfg in zip(bars, [next((r for r in results if r["station"] == station and r["config"] == cfg), None) for cfg in configs], configs):
                if r and not r["passed"]:
                    bar.set_hatch("//")

        ax.set_xticks(list(x))
        ax.set_xticklabels(configs, rotation=35, ha="right", fontsize=7)
        ax.set_ylabel(ylabel)
        ax.set_ylim(bottom=0)
        ax.set_title(ylabel)

    axes[0].legend(loc="upper left", fontsize=7)

    # Add pass/fail legend
    from matplotlib.patches import Patch
    legend_elements = [
        Patch(facecolor="white", edgecolor="gray", label="pass"),
        Patch(facecolor="white", edgecolor="gray", hatch="//", label="fail"),
    ]
    axes[2].legend(handles=legend_elements, loc="upper right", fontsize=7)

    plt.tight_layout()
    fig.savefig(output_path, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {output_path}")


# ── History trend report ──────────────────────────────────────────────────────

def report_history(history_path: str, output_path: str = None, configs_filter: list = None):
    """Plot accuracy trend over commits from history.csv."""
    if not HAS_MPL:
        print("matplotlib not available — skipping report")
        return

    if not os.path.exists(history_path):
        print("No history file found.")
        return

    if output_path is None:
        os.makedirs(REPORTS_DIR, exist_ok=True)
        output_path = os.path.join(REPORTS_DIR, "history.png")

    rows = []
    with open(history_path) as f:
        rows = list(csv.DictReader(f))

    if not rows:
        return

    # Group by config
    all_configs = list(dict.fromkeys(r["config"] for r in rows))
    if configs_filter:
        all_configs = [c for c in all_configs if c in configs_filter]

    plt.rcParams.update(plt_params)
    fig, axes = plt.subplots(1, 2, figsize=(13, 5))
    fig.suptitle("Positioning Accuracy History", fontsize=11)

    for ax, (metric_key, ylabel) in zip(axes, [("3d_rms", "3D RMS (m)"), ("3d_p95", "3D P95 (m)")]):
        for ci, cfg in enumerate(all_configs):
            cfg_rows = [r for r in rows if r["config"] == cfg]
            if not cfg_rows:
                continue
            xs = range(len(cfg_rows))
            ys = [float(r.get(metric_key, 0) or 0) for r in cfg_rows]
            labels = [f"{r['commit'][:6]}\n{r['station'][:6]}" for r in cfg_rows]
            ax.plot(xs, ys, marker="o", markersize=4, label=cfg,
                    color=_COLORS[ci % len(_COLORS)])

        ax.set_ylabel(ylabel)
        ax.set_ylim(bottom=0)
        ax.set_title(ylabel)
        ax.set_xlabel("Run #")

    axes[0].legend(loc="upper right", fontsize=7, ncol=2)
    plt.tight_layout()
    fig.savefig(output_path, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved: {output_path}")
