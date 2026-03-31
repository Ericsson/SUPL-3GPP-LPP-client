"""
Station registry. Each entry defines a reference station.

Fields:
  station   : 9-char IGS name
  date      : "YYYY-MM-DD" or "YYYY-MM-DD:YYYY-MM-DD"
  lat       : approximate latitude (degrees)
  region    : "high-latitude" | "mid-latitude" | "equatorial"
  gnss      : list of available constellations ["gps","gal","glo","bds","qzs"]
  note      : human-readable description

Thresholds in configs.py can be keyed by region.
Configs with gnss_required will only run on datasets that have all required GNSS.
"""

DATASETS = [
    {
        "station": "MAR600SWE",
        "date":    "2026-03-01:2026-03-31",
        "lat":     60.6,
        "region":  "high-latitude",
        "gnss":    ["gps", "glo", "gal", "bds"],
        "note":    "Marsta, Sweden — March 2026",
    },
    {
        "station": "BRUX00BEL",
        "date":    "2026-03-25",
        "lat":     50.8,
        "region":  "mid-latitude",
        "gnss":    ["gps", "glo", "gal"],
        "note":    "Brussels, Belgium",
    },
    {
        "station": "TSKB00JPN",
        "date":    "2026-03-25",
        "lat":     36.1,
        "region":  "mid-latitude",
        "gnss":    ["gps", "glo", "qzs"],
        "note":    "Tsukuba, Japan — has QZSS",
    },
]
