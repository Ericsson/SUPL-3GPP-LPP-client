#!/usr/bin/env python3
"""
Plot position accuracy for N positioning configurations.

CSV format (one file per configuration):
    time_s,sats,dx,dy,dz,d3
    0.0,8,1.23,-0.45,2.10,2.45
    30.0,9,...

Generate from example_spp output (with truth):
    ./build/examples/spp/example_spp nav.rnx obs.rnx TX TY TZ \
      | awk 'NR>1 && NF==9 {print NR-2,"\t",$2,"\t",$3,"\t",$4,"\t",$5,"\t",$6}' \
      | sed '1s/.*/time_s\tsats\tdx\tdy\tdz\td3/' > data/spp.csv

Generate from example_ppp output (SPP columns, with truth):
    ./build/examples/ppp/example_ppp nav.rnx obs.rnx TX TY TZ \
      | awk 'NR>2 && NF==12 {print (NR-3)*30,"\t",$3,"\t",$4,"\t",$5,"\t",$6,"\t",$7}' \
      | sed '1s/.*/time_s\tsats\tdx\tdy\tdz\td3/' > data/spp_from_ppp.csv

Output: positions_analysis.pdf / .png
"""

import sys
import math
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec

# ══════════════════════════════════════════════════════════════════════════════
# CONFIGURATION — edit this section
# ══════════════════════════════════════════════════════════════════════════════

# Truth position in ECEF (meters). Set to None to skip ENU plots.
TRUTH_ECEF = (-3957199.2400, 3310199.6680, 3737711.7080)  # TSKB00JPN

DATASETS = [
    {"label": "SPP el=5°",  "file": "/tmp/spp_jpn_el5.csv"},
    {"label": "SPP el=10°", "file": "/tmp/spp_jpn_el10.csv"},
    {"label": "SPP el=15°", "file": "/tmp/spp_jpn_el15.csv"},
    {"label": "SPP el=20°", "file": "/tmp/spp_jpn_el20.csv"},
]

# ══════════════════════════════════════════════════════════════════════════════

# Color cycle — enough for many datasets
_COLORS = [
    "#2166ac", "#d6604d", "#4dac26", "#8e44ad",
    "#e67e22", "#16a085", "#c0392b", "#2980b9",
]

plt.rcParams.update({
    "font.family":    "serif",
    "font.size":      9,
    "axes.titlesize": 10,
    "axes.labelsize": 9,
    "legend.fontsize":8,
    "xtick.labelsize":8,
    "ytick.labelsize":8,
    "axes.grid":      True,
    "grid.alpha":     0.3,
    "grid.linestyle": "--",
    "lines.linewidth":1.0,
    "figure.dpi":     150,
})


def ecef_to_enu_matrix(x, y, z):
    r   = math.sqrt(x*x + y*y + z*z)
    lat = math.asin(z / r)
    lon = math.atan2(y, x)
    sl, cl = math.sin(lat), math.cos(lat)
    sn, cn = math.sin(lon), math.cos(lon)
    return np.array([
        [-sn,      cn,     0.0],
        [-sl*cn,  -sl*sn,  cl ],
        [ cl*cn,   cl*sn,  sl ],
    ])


def parse_time(s):
    """Parse 'YYYY/MM/DD HH:MM:SS.s' or a plain float into seconds."""
    s = s.strip()
    try:
        return float(s)
    except ValueError:
        pass
    try:
        from datetime import datetime
        fmt = "%Y/%m/%d %H:%M:%S.%f" if "." in s else "%Y/%m/%d %H:%M:%S"
        return datetime.strptime(s, fmt).timestamp()
    except Exception:
        return float("nan")


def load_csv(path):
    """Load a position CSV. Returns dict of numpy arrays.

    Supports the example_spp --output format:
        time,sats,x,y,z,lat,lon,alt,clock[,dx,dy,dz,d3]
    and a minimal format:
        time_s,sats,dx,dy,dz,d3
    Column lookup is by header name.
    """
    rows = []
    header = None
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = [p.strip() for p in line.split(",")]
            if header is None:
                header = [p.lower() for p in parts]
                continue
            rows.append(parts)

    def col(name, row, default="nan"):
        return row[header.index(name)] if name in header else default

    def v(s):
        return float(s) if s not in ("", "-", "nan") else float("nan")

    times = [parse_time(col("time", r) if "time" in header else col("time_s", r)) for r in rows]
    t0    = times[0] if times else 0.0

    data = {
        "time_s": np.array([t - t0 for t in times]),
        "sats":   np.array([int(float(col("sats", r, "0"))) for r in rows]),
        "dx":     np.array([v(col("dx", r)) for r in rows]),
        "dy":     np.array([v(col("dy", r)) for r in rows]),
        "dz":     np.array([v(col("dz", r)) for r in rows]),
        "d3":     np.array([v(col("d3", r)) for r in rows]),
    }
    return data


def ecdf(arr):
    a = arr[~np.isnan(arr)]
    return np.sort(a), np.arange(1, len(a) + 1) / len(a) * 100


def percentile(arr, p):
    a = arr[~np.isnan(arr)]
    return np.percentile(a, p) if len(a) else float("nan")


def main():
    datasets = []
    for i, cfg in enumerate(DATASETS):
        color = cfg.get("color", _COLORS[i % len(_COLORS)])
        try:
            d = load_csv(cfg["file"])
        except FileNotFoundError:
            print(f"WARNING: {cfg['file']} not found, skipping.", file=sys.stderr)
            continue
        datasets.append({"label": cfg["label"], "color": color, "data": d})

    if not datasets:
        print("No datasets loaded. Check DATASETS paths.", file=sys.stderr)
        sys.exit(1)

    has_truth = TRUTH_ECEF is not None
    R = ecef_to_enu_matrix(*TRUTH_ECEF) if has_truth else None

    # Compute ENU for each dataset
    for ds in datasets:
        d = ds["data"]
        if has_truth:
            ecef = np.vstack([d["dx"], d["dy"], d["dz"]])
            enu  = R @ ecef
            d["de"], d["dn"], d["du"] = enu[0], enu[1], enu[2]

    # ── Layout: 5 rows × 7 cols ───────────────────────────────────────────────
    # scatter: cols 0-4 (5 wide), right panels: cols 5-6 (2 wide, 2:1 ratio each)
    if has_truth:
        fig = plt.figure(figsize=(14, 10))
        gs  = gridspec.GridSpec(5, 7, figure=fig, hspace=0.65, wspace=0.55)
        ax_scatter = fig.add_subplot(gs[0:5, 0:5])
        ax_3d      = fig.add_subplot(gs[0, 5:7])
        ax_east    = fig.add_subplot(gs[1, 5:7])
        ax_north   = fig.add_subplot(gs[2, 5:7])
        ax_up      = fig.add_subplot(gs[3, 5:7])
        ax_cdf     = fig.add_subplot(gs[4, 5:7])
    else:
        fig, (ax_3d, ax_cdf) = plt.subplots(1, 2, figsize=(11, 4),
                                             gridspec_kw={"wspace": 0.35})

    # Shared legend handles — collected from first plot pass
    legend_handles = []

    # ── 3D error over time ────────────────────────────────────────────────────
    for ds in datasets:
        d = ds["data"]
        h, = ax_3d.plot(d["time_s"] / 60.0, d["d3"], color=ds["color"], alpha=0.85)
        legend_handles.append(h)
    ax_3d.set_xlabel("Time (min)")
    ax_3d.set_ylabel("3D Error (m)")
    ax_3d.set_title("3D Error")
    ax_3d.set_xlim(left=0)
    ax_3d.set_ylim(bottom=0)

    # ── ENU components ────────────────────────────────────────────────────────
    if has_truth:
        for ax, key, title in [
            (ax_east,  "de", "East"),
            (ax_north, "dn", "North"),
            (ax_up,    "du", "Up"),
        ]:
            for ds in datasets:
                d = ds["data"]
                ax.plot(d["time_s"] / 60.0, d[key], color=ds["color"], alpha=0.85)
            ax.axhline(0, color="k", linewidth=0.5, linestyle=":")
            ax.set_xlabel("Time (min)")
            ax.set_ylabel("Error (m)")
            ax.set_title(f"{title} Component")
            ax.set_xlim(left=0)

        # ── Horizontal scatter ────────────────────────────────────────────────
        for ds in datasets:
            d = ds["data"]
            ax_scatter.scatter(d["de"], d["dn"], s=2, color=ds["color"], alpha=0.4)
        ax_scatter.axhline(0, color="k", linewidth=0.5, linestyle=":")
        ax_scatter.axvline(0, color="k", linewidth=0.5, linestyle=":")
        ax_scatter.set_xlabel("East Error (m)")
        ax_scatter.set_ylabel("North Error (m)")
        ax_scatter.set_title("Horizontal Position Scatter")
        ax_scatter.set_aspect("equal", adjustable="datalim")

    # ── CDF ───────────────────────────────────────────────────────────────────
    for ds in datasets:
        x, y = ecdf(ds["data"]["d3"])
        ax_cdf.step(x, y, color=ds["color"], where="post")
    ax_cdf.axhline(95, color="gray", linewidth=0.7, linestyle="--")
    ax_cdf.set_xlabel("3D Error (m)")
    ax_cdf.set_ylabel("Cumulative (%)")
    ax_cdf.set_title("CDF of 3D Error")
    ax_cdf.set_xlim(left=0)
    ax_cdf.set_ylim(0, 100)

    # ── Legend inside scatter + footer stats ──────────────────────────────────
    ax_scatter.legend(legend_handles, [ds["label"] for ds in datasets],
                      loc="upper right", fontsize=9, framealpha=0.9, markerscale=4)

    stats = "   ".join(
        f"{ds['label']} 95th: {percentile(ds['data']['d3'], 95):.2f} m"
        for ds in datasets
    )
    fig.text(0.5, 0.005, stats, ha="center", fontsize=7.5, color="gray")

    for fmt in ("pdf", "png"):
        out = f"positions_analysis.{fmt}"
        fig.savefig(out, bbox_inches="tight")
        print(f"Saved: {out}")


if __name__ == "__main__":
    main()
