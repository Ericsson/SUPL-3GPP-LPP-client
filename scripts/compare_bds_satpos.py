#!/usr/bin/env python3
"""Compare BDS satellite positions between our SPP and RTKLIB."""
import math, datetime, collections
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

def parse_gps_time(week, tow):
    gps_epoch = datetime.datetime(1980, 1, 6)
    return gps_epoch + datetime.timedelta(weeks=int(week), seconds=float(tow))

def load_our_stat(path):
    """Load our $SAT lines: week,tow,sat,valid,az,el,res,sx,sy,sz,clk,P"""
    data = collections.defaultdict(dict)
    with open(path) as f:
        for line in f:
            if not line.startswith('$SAT'): continue
            p = line.strip().split(',')
            if len(p) < 12: continue
            t = parse_gps_time(p[1], p[2])
            sat = p[3]
            data[t][sat] = {
                'az': float(p[5]), 'el': float(p[6]), 'res': float(p[7]),
                'sx': float(p[8]), 'sy': float(p[9]), 'sz': float(p[10]),
                'clk': float(p[11]), 'P': float(p[12])
            }
    return data

def load_rtklib_satpos(path):
    """Load RTKLIB $RS lines: week,tow,sat,sx,sy,sz,clk,P"""
    data = collections.defaultdict(dict)
    with open(path) as f:
        for line in f:
            if not line.startswith('$RS'): continue
            p = line.strip().split(',')
            if len(p) < 8: continue
            t = parse_gps_time(p[1], p[2])
            sat = p[3]
            data[t][sat] = {
                'sx': float(p[4]), 'sy': float(p[5]), 'sz': float(p[6]),
                'clk': float(p[7]), 'P': float(p[8])
            }
    return data

our  = load_our_stat('/tmp/our_bds.stat')
rtkl = load_rtklib_satpos('/tmp/rtklib_bds_satpos.stat')

# Find common epochs and satellites
common_times = sorted(set(our) & set(rtkl))
print(f"Common epochs: {len(common_times)}")

# Collect per-satellite position differences
sat_diffs = collections.defaultdict(lambda: {'times':[], 'dpos':[], 'dclk':[], 'dP':[]})

for t in common_times:
    for sat in set(our[t]) & set(rtkl[t]):
        o = our[t][sat]
        r = rtkl[t][sat]
        dx = o['sx'] - r['sx']
        dy = o['sy'] - r['sy']
        dz = o['sz'] - r['sz']
        dpos = math.sqrt(dx*dx + dy*dy + dz*dz)
        dclk = (o['clk'] - r['clk']) * 3e8  # convert to meters
        dP   = o['P'] - r['P']
        sat_diffs[sat]['times'].append(t)
        sat_diffs[sat]['dpos'].append(dpos)
        sat_diffs[sat]['dclk'].append(dclk)
        sat_diffs[sat]['dP'].append(dP)

# Find satellites with largest position differences
sat_max = {s: max(v['dpos']) for s, v in sat_diffs.items() if v['dpos']}
top_sats = sorted(sat_max, key=sat_max.get, reverse=True)[:8]
print("\nTop satellites by max position difference:")
for s in top_sats:
    print(f"  {s}: max={sat_max[s]:.1f}m  mean={sum(sat_diffs[s]['dpos'])/len(sat_diffs[s]['dpos']):.1f}m")

# Plot position differences for top satellites
fig, axes = plt.subplots(3, 1, figsize=(14, 10), sharex=True)
fig.suptitle('BDS Satellite Position Differences: Our SPP vs RTKLIB\n(MAR600SWE 2026-03-25)', fontsize=12)

colors = plt.cm.tab10.colors
for idx, sat in enumerate(top_sats[:6]):
    d = sat_diffs[sat]
    c = colors[idx % 10]
    axes[0].plot(d['times'], d['dpos'], color=c, linewidth=0.8, label=sat, alpha=0.8)
    axes[1].plot(d['times'], d['dclk'], color=c, linewidth=0.8, label=sat, alpha=0.8)
    axes[2].plot(d['times'], d['dP'],   color=c, linewidth=0.8, label=sat, alpha=0.8)

axes[0].set_ylabel('|ΔPos| (m)')
axes[0].legend(ncol=3, fontsize=8, loc='upper left')
axes[0].grid(True, alpha=0.3)
axes[1].set_ylabel('ΔClock (m)')
axes[1].grid(True, alpha=0.3)
axes[1].legend(ncol=3, fontsize=8, loc='upper left')
axes[2].set_ylabel('ΔPseudorange (m)')
axes[2].grid(True, alpha=0.3)
axes[2].set_xlabel('Time (UTC)')
axes[2].legend(ncol=3, fontsize=8, loc='upper left')

for ax in axes:
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
    ax.xaxis.set_major_locator(mdates.HourLocator(interval=2))
fig.autofmt_xdate()

outpath = '/home/ewasjon/Documents/SUPL-3GPP-LPP-client/bds_satpos_diff.png'
plt.tight_layout()
plt.savefig(outpath, dpi=150, bbox_inches='tight')
plt.close()
print(f"\nSaved {outpath}")

# Also print stats for a specific bad epoch (around 04:00-05:00)
bad_time = datetime.datetime(2026, 3, 25, 4, 30, 0)
closest = min(common_times, key=lambda t: abs((t - bad_time).total_seconds()))
print(f"\nAt {closest.strftime('%H:%M:%S')} (bad epoch):")
print(f"  {'SAT':6s}  {'|ΔPos|':8s}  {'ΔClk(m)':10s}  {'ΔP(m)':8s}")
for sat in sorted(set(our[closest]) & set(rtkl[closest])):
    o = our[closest][sat]
    r = rtkl[closest][sat]
    dx,dy,dz = o['sx']-r['sx'], o['sy']-r['sy'], o['sz']-r['sz']
    dpos = math.sqrt(dx*dx+dy*dy+dz*dz)
    dclk = (o['clk']-r['clk'])*3e8
    dP   = o['P']-r['P']
    print(f"  {sat:6s}  {dpos:8.2f}  {dclk:10.2f}  {dP:8.2f}")
