"""
Configuration presets.

Fields:
  engine        : "spp" or "ppp"
  flags         : CLI flags passed to the engine binary
  gnss_required : list of GNSS that must be present in the dataset (e.g. ["gal"])
                  Empty list = runs on all datasets.
  thresholds    : dict of metric -> limit, optionally keyed by region.
                  Use "default" for the fallback.
                  Metrics: p95_3d, rms_3d, availability (higher-is-better)

Region keys: "high-latitude", "mid-latitude", "equatorial"

Add a new entry here to run it against all compatible stations automatically.
"""

# Shared threshold templates
_T_SPP = {
    "default":       {"p95_3d": 12.0, "availability": 0.90},
    "high-latitude": {"p95_3d": 15.0, "availability": 0.88},
    "equatorial":    {"p95_3d": 18.0, "availability": 0.88},
}
_T_SPP_STATIC = {
    "default":       {"p95_3d": 6.0,  "availability": 0.92},
    "high-latitude": {"p95_3d": 8.0,  "availability": 0.90},
    "equatorial":    {"p95_3d": 10.0, "availability": 0.90},
}

CONFIGS = {
    # ── Single-constellation SPP ──────────────────────────────────────────────
    "spp_gps": {
        "engine":        "spp",
        "flags":         ["--gps"],
        "gnss_required": ["gps"],
        "thresholds":    _T_SPP,
    },
    "spp_gal": {
        "engine":        "spp",
        "flags":         ["--gal"],
        "gnss_required": ["gal"],
        "thresholds":    _T_SPP,
    },
    "spp_glo": {
        "engine":        "spp",
        "flags":         ["--glo"],
        "gnss_required": ["glo"],
        "thresholds":    _T_SPP,
    },
    "spp_bds": {
        "engine":        "spp",
        "flags":         ["--bds"],
        "gnss_required": ["bds"],
        "timeout_per_day": 180,  # BDS is slower (~2min/day) due to many satellites
        "thresholds": {
            "default":       {"p95_3d": 25.0, "availability": 0.85},
            "high-latitude": {"p95_3d": 30.0, "availability": 0.82},
        },
    },
    "spp_qzs": {
        "engine":        "spp",
        "flags":         ["--qzs"],
        "gnss_required": ["qzs"],
        "thresholds": {
            "default": {"p95_3d": 20.0, "availability": 0.80},
        },
    },

    # ── Multi-constellation SPP ───────────────────────────────────────────────
    "spp_gps_gal": {
        "engine":        "spp",
        "flags":         ["--gps", "--gal"],
        "gnss_required": ["gps", "gal"],
        "thresholds":    _T_SPP,
    },
    "spp_gps_gal_bds": {
        "engine":        "spp",
        "flags":         ["--gps", "--gal", "--bds"],
        "gnss_required": ["gps", "gal", "bds"],
        "thresholds":    _T_SPP,
    },

    # ── Kalman static ─────────────────────────────────────────────────────────
    "spp_gps_static": {
        "engine":        "spp",
        "flags":         ["--gps", "--filter", "static"],
        "gnss_required": ["gps"],
        "thresholds":    _T_SPP_STATIC,
    },
    "spp_gps_gal_static": {
        "engine":        "spp",
        "flags":         ["--gps", "--gal", "--filter", "static"],
        "gnss_required": ["gps", "gal"],
        "thresholds":    _T_SPP_STATIC,
    },

    # ── PPP ───────────────────────────────────────────────────────────────────
    "ppp_gps": {
        "engine":        "ppp",
        "flags":         [],
        "gnss_required": ["gps"],
        "thresholds": {
            "default": {"p95_3d": 5.0, "availability": 0.85},
        },
    },
}
