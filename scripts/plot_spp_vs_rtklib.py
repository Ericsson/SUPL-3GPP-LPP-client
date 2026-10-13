#!/usr/bin/env python3
"""Scatter plot: SPP vs RTKLIB 3D error per epoch, one image per GNSS."""
import math, datetime, collections
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

TRUTH = (2998189.7141, 931451.5896, 5533398.4769)

def parse_time(s):
    return datetime.datetime.strptime(s.strip()[:19], "%Y/%m/%d %H:%M:%S")

def round30(dt):
    r = round(dt.second / 30) * 30
    return dt.replace(second=0, microsecond=0) + datetime.timedelta(seconds=r)

def load_spp(path):
    data = {}
    with open(path) as f:
        next(f)
        for line in f:
            p = line.strip().split(',')
            if len(p) < 5: continue
            try:
                t = round30(parse_time(p[0]))
                x, y, z = float(p[2]), float(p[3]), float(p[4])
                d3 = math.sqrt((x-TRUTH[0])**2+(y-TRUTH[1])**2+(z-TRUTH[2])**2)
                data[t] = d3
            except: pass
    return data

def load_rtklib(path):
    data = {}
    with open(path) as f:
        for line in f:
            if line.startswith('%'): continue
            p = line.split()
            if len(p) < 6: continue
            try:
                if int(p[5]) != 5: continue
                t = round30(parse_time(p[0]+' '+p[1]))
                x, y, z = float(p[2]), float(p[3]), float(p[4])
                d3 = math.sqrt((x-TRUTH[0])**2+(y-TRUTH[1])**2+(z-TRUTH[2])**2)
                data[t] = d3
            except: pass
    return data

systems = [
    ('gps',  'GPS',     '#1f77b4'),
    ('gal',  'Galileo', '#ff7f0e'),
    ('glo',  'GLONASS', '#2ca02c'),
    ('bds',  'BDS',     '#d62728'),
]

for name, label, color in systems:
    spp = load_spp(f'/tmp/spp_{name}.csv')
    rtk = load_rtklib(f'/tmp/rtklib_{name}.pos')

    common = sorted(set(spp) & set(rtk))
    if not common:
        print(f"No common epochs for {name}")
        continue

    sv = [spp[t] for t in common]
    rv = [rtk[t] for t in common]

    spp_mean = sum(sv)/len(sv)
    rtk_mean = sum(rv)/len(rv)
    spp_p95  = sorted(sv)[int(0.95*(len(sv)-1))]
    rtk_p95  = sorted(rv)[int(0.95*(len(rv)-1))]

    fig, axes = plt.subplots(1, 2, figsize=(14, 6))
    fig.suptitle(f'SPP vs RTKLIB — {label} only\nMAR600SWE 2026-03-25, el≥10°, Klobuchar+Saastamoinen', fontsize=12)

    # Left: scatter plot SPP vs RTKLIB per epoch
    ax = axes[0]
    ax.scatter(rv, sv, s=2, alpha=0.3, color=color)
    lim = max(max(sv), max(rv)) * 1.05
    ax.plot([0, lim], [0, lim], 'k--', linewidth=0.8, label='1:1 line')
    ax.set_xlabel(f'RTKLIB 3D error (m)')
    ax.set_ylabel(f'Our SPP 3D error (m)')
    ax.set_title(f'Scatter: {len(common)} epochs\nSPP mean={spp_mean:.2f}m  RTKLIB mean={rtk_mean:.2f}m')
    ax.legend(fontsize=8)
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, lim); ax.set_ylim(0, lim)

    # Right: time series
    ax2 = axes[1]
    ax2.plot(common, sv, color=color, alpha=0.7, linewidth=0.6,
             label=f'Our SPP  (mean={spp_mean:.2f}m, 95th={spp_p95:.2f}m)')
    ax2.plot(common, rv, color='black', alpha=0.5, linewidth=0.6,
             label=f'RTKLIB   (mean={rtk_mean:.2f}m, 95th={rtk_p95:.2f}m)')
    ax2.set_ylabel('3D Error (m)')
    ax2.set_xlabel('Time (UTC)')
    ax2.set_title('Time series')
    ax2.legend(fontsize=8, loc='upper right')
    ax2.grid(True, alpha=0.3)
    ax2.set_ylim(bottom=0)
    ax2.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
    ax2.xaxis.set_major_locator(mdates.HourLocator(interval=4))
    fig.autofmt_xdate()

    outpath = f'/home/ewasjon/Documents/SUPL-3GPP-LPP-client/spp_vs_rtklib_{name}.png'
    plt.tight_layout()
    plt.savefig(outpath, dpi=150, bbox_inches='tight')
    plt.close()
    print(f"Saved {outpath}  |  SPP mean={spp_mean:.2f}m p95={spp_p95:.2f}m  |  RTKLIB mean={rtk_mean:.2f}m p95={rtk_p95:.2f}m")
