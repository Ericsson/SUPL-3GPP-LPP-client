"""
Append/load historical results per commit.
"""

import csv
import os
import subprocess
from datetime import datetime

HISTORY_FILE = os.path.join(os.path.dirname(__file__), "history.csv")

HISTORY_FIELDS = [
    "timestamp", "commit", "station", "config",
    "n_epochs", "availability",
    "3d_mean", "3d_rms", "3d_p95", "3d_min", "3d_max",
    "h_mean", "h_rms", "h_p95",
    "v_mean", "v_rms", "v_p95",
    "pdop_mean", "hdop_mean", "vdop_mean",
    "uere", "sats_mean",
]


def _git_commit() -> str:
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=os.path.dirname(__file__), stderr=subprocess.DEVNULL
        ).decode().strip()
    except Exception:
        return "unknown"


def record(station: str, config: str, metrics: dict):
    exists = os.path.exists(HISTORY_FILE)
    with open(HISTORY_FILE, "a", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=HISTORY_FIELDS, extrasaction="ignore")
        if not exists:
            writer.writeheader()
        row = {
            "timestamp": datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ"),
            "commit":    _git_commit(),
            "station":   station,
            "config":    config,
        }
        row.update(metrics)
        writer.writerow(row)


def load() -> list:
    if not os.path.exists(HISTORY_FILE):
        return []
    with open(HISTORY_FILE) as f:
        return list(csv.DictReader(f))


def print_trend(config: str = None, station: str = None, n: int = 10):
    rows = load()
    if config:
        rows = [r for r in rows if r["config"] == config]
    if station:
        rows = [r for r in rows if r["station"] == station]
    rows = rows[-n:]
    if not rows:
        print("No history found.")
        return
    print(f"\n{'Timestamp':<22} {'Commit':<8} {'Station':<12} {'Config':<22} {'3D RMS':>8} {'P95':>8} {'UERE':>6}")
    print("-" * 95)
    for r in rows:
        print(f"{r.get('timestamp',''):<22} {r.get('commit',''):<8} {r.get('station',''):<12} "
              f"{r.get('config',''):<22} {float(r.get('3d_rms',0) or 0):>8.3f} "
              f"{float(r.get('3d_p95',0) or 0):>8.3f} {float(r.get('uere',0) or 0):>6.3f}")
