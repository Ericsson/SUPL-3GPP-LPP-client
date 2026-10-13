#!/usr/bin/env python3
"""
Optimal encoding of an arbitrary high-precision phase range into RTCM MSM4/5.

MSM4/5 phase range structure (all in meters, converted from ms*c units):

  PhaseRange = (integer_ms * C_ms) + (rough_range_int * 2^-10 * C_ms) + (fine_int * 2^-29 * C_ms)

  DF397  integer_ms        8-bit unsigned  [0..254]    1 LSB = c/1000  = 299,792 m
  DF398  rough_range      10-bit unsigned  [0..1023]   1 LSB = c/1024  = 293 m     (2^-10 ms*c)
  DF401  fine_phase_range 22-bit signed    [-2^21..2^21-1]  1 LSB = 0.558 mm  (2^-29 ms*c)
                          physical range: ±2^21 * 2^-29 * C_ms = ±1171 m

Key insight:
  rough_range covers ONE integer_ms interval: [0, 1023 * 293m) = [0, ~299km)
  fine_phase_range covers ±1171m around the rough value.
  Since rough has 293m steps, and fine covers ±1171m (= ±4 rough steps),
  there is plenty of headroom — no overflow from encoding structure.

Question: for an arbitrary phase range with sub-mm precision,
  can we choose N wavelengths to add to fine_phase_range to improve precision?
"""

import numpy as np
import matplotlib.pyplot as plt

C     = 299_792_458.0
C_MS  = C * 1e-3            # 299792.458 m  (1 ms * c)

L1_LAMBDA = C / 1_575_420_000.0  # 0.190293 m
L2_LAMBDA = C / 1_227_600_000.0  # 0.244210 m
L5_LAMBDA = C / 1_176_450_000.0  # 0.254828 m

INT_MS_SCALE  = C_MS              # 299792.458 m per integer_ms count
ROUGH_SCALE   = 2**-10 * C_MS    # 292.766 m per LSB
ROUGH_INT_MAX = 1023

FINE_SCALE    = 2**-29 * C_MS    # 0.000558 m = 0.558 mm per LSB
FINE_INT_MAX  =  2_097_151        # 2^21 - 1
FINE_INT_MIN  = -2_097_152        # -2^21
FINE_RANGE    = FINE_INT_MAX * FINE_SCALE  # ±1171.06 m

print("=== MSM4/5 field summary ===")
print(f"  integer_ms  : 1 LSB = {INT_MS_SCALE/1000:.3f} km")
print(f"  rough_range : 1 LSB = {ROUGH_SCALE:.3f} m,  covers [0, {ROUGH_INT_MAX*ROUGH_SCALE/1000:.3f} km)")
print(f"  fine_phase  : 1 LSB = {FINE_SCALE*1000:.4f} mm,  covers ±{FINE_RANGE:.2f} m")
print(f"  L1 lambda   = {L1_LAMBDA*100:.4f} cm  ({FINE_RANGE/L1_LAMBDA:.0f} wavelengths fit in fine field)\n")


def encode_msm45(phase_m, wl_offset=None):
    """
    Encode phase_m into (integer_ms, rough_int, fine_int).
    If wl_offset given, subtract nearest N wavelengths from fine residual.
    Returns decoded value and error.
    """
    int_ms   = int(phase_m / INT_MS_SCALE)
    int_ms   = max(0, min(254, int_ms))
    r1       = phase_m - int_ms * INT_MS_SCALE

    rough_int = round(r1 / ROUGH_SCALE)          # ROUND, not floor
    rough_int = max(0, min(ROUGH_INT_MAX, rough_int))
    r2        = r1 - rough_int * ROUGH_SCALE      # fine residual, typically ±146 m

    N = 0
    if wl_offset is not None:
        N  = round(r2 / wl_offset)
        r2 = r2 - N * wl_offset

    fine_int = round(r2 / FINE_SCALE)
    overflow = fine_int < FINE_INT_MIN or fine_int > FINE_INT_MAX
    fine_int = max(FINE_INT_MIN, min(FINE_INT_MAX, fine_int))

    decoded  = int_ms * INT_MS_SCALE + rough_int * ROUGH_SCALE + fine_int * FINE_SCALE + N * (wl_offset or 0)
    return decoded, (decoded - phase_m) * 1000, overflow


# ── 1. Overflow analysis ──────────────────────────────────────────────────────
print("=== 1. Overflow analysis ===")
print(f"  rough step  = {ROUGH_SCALE:.3f} m")
print(f"  fine range  = ±{FINE_RANGE:.2f} m")
print(f"  With ROUND on rough: fine residual ∈ [±{ROUGH_SCALE/2:.2f} m]  << fine range")
print(f"  => fine field NEVER overflows for any well-formed phase range\n")


# ── 2. Precision vs. magnitude ────────────────────────────────────────────────
print("=== 2. Does magnitude of the fine residual affect precision? ===\n")

np.random.seed(0)
n = 100_000
# Arbitrary phase ranges: 0 to 25,000 km (typical GNSS pseudorange territory)
phases = np.random.uniform(0, 25_000_000.0, n)

results = {}
for label, wl in [("direct",       None),
                  ("L1 reduction", L1_LAMBDA),
                  ("L2 reduction", L2_LAMBDA),
                  ("L5 reduction", L5_LAMBDA)]:
    errs = np.array([encode_msm45(p, wl)[1] for p in phases])
    results[label] = errs
    rms = np.sqrt(np.mean(errs**2))
    print(f"  {label:<20}  RMS = {rms:.6f} mm  (theoretical = {FINE_SCALE*1000 / (2*np.sqrt(3)):.6f} mm)")

print(f"\n  Fine field LSB = {FINE_SCALE*1000:.4f} mm")
print(f"  All strategies give the same precision — fixed-point is magnitude-independent.\n")


# ── 3. The residual distribution (what actually uses the fine field) ──────────
print("=== 3. Fine residual distribution ===\n")

fine_residuals_direct = []
fine_residuals_l1     = []

for p in phases[:20_000]:
    int_ms = int(p / INT_MS_SCALE)
    r1     = p - int_ms * INT_MS_SCALE
    rough  = round(r1 / ROUGH_SCALE)
    r2     = r1 - rough * ROUGH_SCALE

    fine_residuals_direct.append(r2 * 1000)  # mm

    N  = round(r2 / L1_LAMBDA)
    fine_residuals_l1.append((r2 - N * L1_LAMBDA) * 1000)

dr = np.array(fine_residuals_direct)
lr = np.array(fine_residuals_l1)

print(f"  Direct:       range [{dr.min():.2f}, {dr.max():.2f}] mm,  RMS = {np.sqrt(np.mean(dr**2)):.2f} mm")
print(f"  L1-reduced:   range [{lr.min():.3f}, {lr.max():.3f}] mm,  RMS = {np.sqrt(np.mean(lr**2)):.3f} mm")
print(f"\n  Direct uses {abs(dr.max()-dr.min()):.0f} mm of the ±{FINE_RANGE*1000:.0f} mm fine range (~{100*abs(dr.max()-dr.min())/(2*FINE_RANGE*1000):.4f}%)")
print(f"  L1-reduced uses only ±{L1_LAMBDA/2*1000:.1f} mm  (~{100*(L1_LAMBDA)/(2*FINE_RANGE*1000):.5f}% of fine range)\n")


# ── 4. Plot ───────────────────────────────────────────────────────────────────
fig, axes = plt.subplots(1, 3, figsize=(15, 4))
fig.suptitle("MSM4/5 fine phase range residuals for arbitrary high-precision input", fontsize=12)

configs = [
    ("Direct (ROUND rough)", np.array(fine_residuals_direct), "steelblue"),
    ("L1-reduced (±λ/2)",    np.array(fine_residuals_l1),     "darkorange"),
]

# Also show quantization error
for ax, (label, data, color) in zip(axes[:2], configs):
    ax.hist(data, bins=300, color=color, alpha=0.8)
    ax.set_title(label)
    ax.set_xlabel("Fine residual [mm]")
    ax.set_ylabel("Count")
    rms = np.sqrt(np.mean(data**2))
    ax.text(0.05, 0.95, f"RMS = {rms:.3f} mm", transform=ax.transAxes,
            va='top', fontsize=9, bbox=dict(boxstyle='round', fc='white', alpha=0.8))

# Quantization error comparison
ax = axes[2]
q_direct = np.array([encode_msm45(p)[1]       for p in phases[:20_000]])
q_l1     = np.array([encode_msm45(p, L1_LAMBDA)[1] for p in phases[:20_000]])
ax.hist(q_direct, bins=100, alpha=0.6, label=f"Direct  (RMS={np.sqrt(np.mean(q_direct**2)):.4f} mm)", color='steelblue')
ax.hist(q_l1,     bins=100, alpha=0.6, label=f"L1 red. (RMS={np.sqrt(np.mean(q_l1**2)):.4f} mm)",     color='darkorange')
ax.set_title("Quantization error")
ax.set_xlabel("Encoding error [mm]")
ax.set_ylabel("Count")
ax.legend(fontsize=8)

plt.tight_layout()
plt.savefig("/home/ewasjon/Documents/SUPL-3GPP-LPP-client/scripts/msm45_optimal_encoding.png", dpi=120)
print("  Plot saved to scripts/msm45_optimal_encoding.png\n")


print("=== CONCLUSION ===")
print(f"""
Optimal MSM4/5 encoding of an arbitrary high-precision phase range:

  1. integer_ms = floor(phase / C_ms)                    [{INT_MS_SCALE/1000:.0f} km resolution]
  2. rough_int  = ROUND(residual / {ROUGH_SCALE:.3f})      [293 m resolution, use ROUND not floor]
  3. fine_int   = round(residual / {FINE_SCALE*1000:.4f} mm)

Wavelength offsetting (adding N*lambda to fine_int):
  - Does NOT improve precision (fixed-point quantization error is always ±{FINE_SCALE/2*1000:.3f} mm)
  - Does NOT prevent overflow (rough truncation ±{ROUGH_SCALE/2:.0f} m << fine range ±{FINE_RANGE:.0f} m)
  - DOES reduce the magnitude of the fine residual from ±{ROUGH_SCALE/2*1000:.0f} mm to ±{L1_LAMBDA/2*1000:.0f} mm
    but this is cosmetic — it doesn't affect the encoded integer or the error

The fundamental precision limit is the fine field LSB = {FINE_SCALE*1000:.4f} mm.
No encoding trick can improve beyond that.
""")
