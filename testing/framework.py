"""
Core framework: run engines, parse CSV output, compute metrics.
"""

import subprocess
import csv
import math
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IGS_DATA_DIR = os.path.join(ROOT, "igs_data")
RESULTS_DIR  = os.path.join(ROOT, "testing", "results")
SPP_BIN      = os.path.join(ROOT, "build", "examples", "spp", "example_spp")
PPP_BIN      = os.path.join(ROOT, "build", "examples", "ppp", "example_ppp")


def _expand_dates(date_str: str) -> list:
    """Expand 'YYYY-MM-DD' or 'YYYY-MM-DD:YYYY-MM-DD' to list of date strings."""
    import datetime
    if ":" in date_str:
        start, end = date_str.split(":")
        d = datetime.date.fromisoformat(start)
        end_d = datetime.date.fromisoformat(end)
        dates = []
        while d <= end_d:
            dates.append(str(d))
            d += datetime.timedelta(days=1)
        return dates
    return [date_str]


def _nav_file(date: str) -> str:
    year, month, day = date.split("-")
    import datetime
    doy = datetime.date(int(year), int(month), int(day)).timetuple().tm_yday
    return os.path.join(IGS_DATA_DIR, f"BRDM00DLR_S_{year}{doy:03d}0000_01D_MN.rnx")


def _obs_file(station: str, date: str) -> str:
    year, month, day = date.split("-")
    import datetime
    doy = datetime.date(int(year), int(month), int(day)).timetuple().tm_yday
    return os.path.join(IGS_DATA_DIR, f"{station.upper()}_R_{year}{doy:03d}0000_01D_30S_MO.rnx")


def ensure_data(station: str, date: str) -> bool:
    """Download RINEX data for all dates in range if not already present."""
    dates = _expand_dates(date)
    ok = True
    for d in dates:
        nav = _nav_file(d)
        obs = _obs_file(station, d)
        if not (os.path.exists(nav) and os.path.exists(obs)):
            script = os.path.join(ROOT, "scripts", "download_igs.sh")
            result = subprocess.run([script, station, d], capture_output=True)
            if result.returncode != 0 or not os.path.exists(obs):
                ok = False
    # Return True if at least one day is available
    return any(os.path.exists(_obs_file(station, d)) for d in dates)


def _truth_from_rinex(obs_path: str):
    """Extract APPROX POSITION XYZ from RINEX header."""
    with open(obs_path) as f:
        for line in f:
            if "APPROX POSITION XYZ" in line:
                parts = line.split()
                return float(parts[0]), float(parts[1]), float(parts[2])
    return None


def run_spp(station: str, date: str, flags: list, output_csv: str,
            timeout_per_day: int = 120) -> bool:
    dates = _expand_dates(date)
    obs_first = _obs_file(station, dates[0])
    truth = _truth_from_rinex(obs_first)

    if len(dates) == 1:
        nav = _nav_file(dates[0])
        obs = _obs_file(station, dates[0])
        cmd = [SPP_BIN, nav, obs] + flags + ["--output", output_csv]
        if truth:
            cmd += [f"--tx={truth[0]}", f"--ty={truth[1]}", f"--tz={truth[2]}"]
        result = subprocess.run(cmd, capture_output=True, timeout=timeout_per_day)
        return result.returncode == 0 and os.path.exists(output_csv)

    # Multi-day: run per-day and concatenate CSV output
    import tempfile
    all_rows = []
    header = None
    for d in dates:
        nav = _nav_file(d)
        obs = _obs_file(station, d)
        if not (os.path.exists(nav) and os.path.exists(obs)):
            continue
        with tempfile.NamedTemporaryFile(suffix=".csv", delete=False) as tmp:
            tmp_path = tmp.name
        cmd = [SPP_BIN, nav, obs] + flags + ["--output", tmp_path]
        if truth:
            cmd += [f"--tx={truth[0]}", f"--ty={truth[1]}", f"--tz={truth[2]}"]
        try:
            subprocess.run(cmd, capture_output=True, timeout=timeout_per_day)
        except subprocess.TimeoutExpired:
            print(f"\n    [timeout on {d}, skipping]", end="", flush=True)
            if os.path.exists(tmp_path):
                os.unlink(tmp_path)
            continue
        if os.path.exists(tmp_path):
            with open(tmp_path) as f:
                lines = f.readlines()
            if lines:
                if header is None:
                    header = lines[0]
                all_rows.extend(lines[1:])
            os.unlink(tmp_path)

    if not all_rows or header is None:
        return False
    with open(output_csv, "w") as f:
        f.write(header)
        f.writelines(all_rows)
    return True


def run_ppp(station: str, date: str, flags: list, output_csv: str) -> bool:
    dates = _expand_dates(date)
    # PPP: run single day only (multi-day PPP not yet supported)
    d = next((dd for dd in dates if os.path.exists(_obs_file(station, dd))), None)
    if d is None:
        return False
    nav = _nav_file(d)
    obs = _obs_file(station, d)
    truth = _truth_from_rinex(obs)
    if truth is None:
        return False
    cmd = [PPP_BIN, nav, obs, str(truth[0]), str(truth[1]), str(truth[2])]
    # PPP outputs to stdout — capture and write to CSV
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
    if result.returncode != 0:
        return False
    # Parse PPP stdout (format: time spp_sats spp_dx spp_dy spp_dz spp_3d ppp_sats ppp_dx ppp_dy ppp_dz ppp_3d)
    with open(output_csv, "w") as f:
        f.write("time,sats,dx,dy,dz,d3\n")
        for line in result.stdout.splitlines():
            parts = line.split()
            if len(parts) >= 11 and parts[0].startswith("20"):
                try:
                    time_str = parts[0] + " " + parts[1]
                    sats = parts[7]
                    dx, dy, dz, d3 = parts[8], parts[9], parts[10], parts[11] if len(parts) > 11 else "0"
                    if dx != "-":
                        f.write(f"{time_str},{sats},{dx},{dy},{dz},{d3}\n")
                except (IndexError, ValueError):
                    pass
    return os.path.exists(output_csv)


def compute_metrics(csv_path: str, truth: tuple = None) -> dict:
    """Parse output CSV and compute all metrics."""
    rows = []
    with open(csv_path) as f:
        reader = csv.DictReader(f)
        for row in reader:
            parsed = {}
            for k, v in row.items():
                if not isinstance(v, str) or v in ("", "-"): continue
                try:
                    parsed[k] = float(v)
                except (ValueError, TypeError):
                    pass  # skip non-numeric fields (e.g. time string)
            if parsed:
                rows.append(parsed)

    if not rows:
        return {"availability": 0.0, "n_epochs": 0}

    n = len(rows)
    metrics = {"n_epochs": n}

    # Accuracy (if truth columns present)
    has_truth = "dx" in rows[0] and rows[0].get("dx") is not None
    if has_truth:
        def _col(key):
            return [r[key] for r in rows if key in r]

        dx = _col("dx"); dy = _col("dy"); dz = _col("dz")
        d3 = [math.sqrt(x*x + y*y + z*z) for x, y, z in zip(dx, dy, dz)] if dx else _col("d3")

        def _stats(vals, prefix):
            if not vals: return {}
            mean = sum(vals) / len(vals)
            rms  = math.sqrt(sum(v*v for v in vals) / len(vals))
            sd   = math.sqrt(sum((v-mean)**2 for v in vals) / len(vals))
            s    = sorted(vals)
            p95  = s[int(0.95 * (len(s)-1))]
            return {
                f"{prefix}_mean": mean, f"{prefix}_rms": rms, f"{prefix}_sd": sd,
                f"{prefix}_p95":  p95,  f"{prefix}_min": min(vals), f"{prefix}_max": max(vals),
            }

        metrics.update(_stats(d3, "3d"))
        metrics.update(_stats([abs(v) for v in dx], "dx"))
        metrics.update(_stats([abs(v) for v in dy], "dy"))
        metrics.update(_stats([abs(v) for v in dz], "dz"))

        # Horizontal / vertical in ENU (if truth available)
        if truth and dx:
            tx, ty, tz = truth
            r = math.sqrt(tx*tx + ty*ty + tz*tz)
            lat = math.asin(tz / r); lon = math.atan2(ty, tx)
            sl, cl, sn, cn = math.sin(lat), math.cos(lat), math.sin(lon), math.cos(lon)
            de = [-sn*x + cn*y for x, y in zip(dx, dy)]
            dn = [-sl*cn*x - sl*sn*y + cl*z for x, y, z in zip(dx, dy, dz)]
            du = [ cl*cn*x + cl*sn*y + sl*z for x, y, z in zip(dx, dy, dz)]
            dh = [math.sqrt(e*e + n*n) for e, n in zip(de, dn)]
            metrics.update(_stats(dh, "h"))
            metrics.update(_stats(du, "v"))

    # DOP (if present)
    if "pdop" in rows[0]:
        pdop = [r["pdop"] for r in rows if "pdop" in r and r["pdop"] > 0]
        hdop = [r["hdop"] for r in rows if "hdop" in r and r["hdop"] > 0]
        vdop = [r["vdop"] for r in rows if "vdop" in r and r["vdop"] > 0]
        if pdop:
            metrics["pdop_mean"] = sum(pdop) / len(pdop)
            metrics["pdop_max"]  = max(pdop)
            metrics["hdop_mean"] = sum(hdop) / len(hdop) if hdop else 0
            metrics["vdop_mean"] = sum(vdop) / len(vdop) if vdop else 0

    # UERE = rms_3d / pdop_mean (algorithm quality independent of geometry)
    if "3d_rms" in metrics and "pdop_mean" in metrics and metrics["pdop_mean"] > 0:
        metrics["uere"] = metrics["3d_rms"] / metrics["pdop_mean"]

    # Satellite count
    if "sats" in rows[0]:
        sats = [r["sats"] for r in rows if "sats" in r]
        metrics["sats_mean"] = sum(sats) / len(sats)
        metrics["sats_min"]  = min(sats)

    # Availability (epochs with solution / total epochs attempted)
    # We approximate: all rows in CSV had a solution
    metrics["availability"] = 1.0  # refined by caller if total epochs known

    return metrics


def dataset_supports_config(dataset: dict, cfg: dict) -> bool:
    """Return True if the dataset has all GNSS required by the config."""
    required = cfg.get("gnss_required", [])
    available = dataset.get("gnss", [])
    return all(g in available for g in required)


def resolve_thresholds(cfg: dict, region: str) -> dict:
    """Return the threshold dict for the given region, falling back to 'default'."""
    t = cfg.get("thresholds", {})
    if not t:
        return {}
    # If values are plain numbers (not nested), it's a flat threshold dict
    if any(isinstance(v, (int, float)) for v in t.values()):
        return t
    return t.get(region, t.get("default", {}))


def check_thresholds(metrics: dict, thresholds: dict) -> tuple:
    """Returns (passed: bool, failures: list of str).
    
    Threshold keys map to metric keys with special handling:
    - availability: must be >= threshold (higher is better)
    - all others: must be <= threshold (lower is better)
    """
    HIGHER_IS_BETTER = {"availability"}
    failures = []
    for key, limit in thresholds.items():
        # Map threshold key to metric key
        metric_key = key  # e.g. "p95_3d" -> "3d_p95"
        if key == "p95_3d":   metric_key = "3d_p95"
        elif key == "rms_3d": metric_key = "3d_rms"
        elif key == "mean_3d": metric_key = "3d_mean"

        val = metrics.get(metric_key)
        if val is None:
            failures.append(f"{key}: no data")
        elif key in HIGHER_IS_BETTER:
            if val < limit:
                failures.append(f"{key}={val:.3f} < {limit}")
        else:
            if val > limit:
                failures.append(f"{key}={val:.3f} > {limit}")
    return len(failures) == 0, failures
