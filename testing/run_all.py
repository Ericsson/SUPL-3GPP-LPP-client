#!/usr/bin/env python3
"""
Main entry point for the GNSS positioning test framework.

Usage:
  python3 testing/run_all.py                          # run all compatible tests
  python3 testing/run_all.py --config spp_gps         # one config
  python3 testing/run_all.py --station MAR600SWE       # one station
  python3 testing/run_all.py --record                  # save to history
  python3 testing/run_all.py --history                 # show trend table
  python3 testing/run_all.py --report                  # generate PNG reports
  python3 testing/run_all.py --record --report         # run + record + report
"""

import argparse
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from testing.datasets  import DATASETS
from testing.configs   import CONFIGS
from testing.framework import (ensure_data, run_spp, run_ppp,
                                compute_metrics, check_thresholds,
                                dataset_supports_config, resolve_thresholds,
                                _obs_file, _truth_from_rinex, _expand_dates,
                                RESULTS_DIR)
import testing.history as history
import testing.report  as report

os.makedirs(RESULTS_DIR, exist_ok=True)

PASS = "\033[32m✓\033[0m"
FAIL = "\033[31m✗\033[0m"
SKIP = "\033[33m-\033[0m"


def run_one(dataset: dict, config_name: str, cfg: dict, do_record: bool) -> dict:
    station = dataset["station"]
    date    = dataset["date"]
    region  = dataset.get("region", "default")
    engine  = cfg["engine"]
    flags   = cfg["flags"]

    print(f"  {station} / {config_name} ... ", end="", flush=True)

    if not ensure_data(station, date):
        print(f"{SKIP} data unavailable")
        return {"status": "skip"}

    # Find first available date for truth
    first_date = next(
        (d for d in _expand_dates(date) if os.path.exists(_obs_file(station, d))),
        None
    )
    if first_date is None:
        print(f"{SKIP} no obs files")
        return {"status": "skip"}

    obs   = _obs_file(station, first_date)
    truth = _truth_from_rinex(obs)

    out_csv = os.path.join(RESULTS_DIR, f"{station}_{config_name}.csv")

    ok = False
    if engine == "spp":
        ok = run_spp(station, date, flags, out_csv,
                     timeout_per_day=cfg.get("timeout_per_day", 120))
    elif engine == "ppp":
        ok = run_ppp(station, date, flags, out_csv)

    if not ok:
        print(f"{FAIL} engine failed")
        return {"status": "error"}

    metrics    = compute_metrics(out_csv, truth)
    thresholds = resolve_thresholds(cfg, region)
    passed, failures = check_thresholds(metrics, thresholds)

    n    = metrics.get("n_epochs", 0)
    rms  = metrics.get("3d_rms",   float("nan"))
    p95  = metrics.get("3d_p95",   float("nan"))
    pdop = metrics.get("pdop_mean",float("nan"))
    uere = metrics.get("uere",     float("nan"))
    sats = metrics.get("sats_mean",float("nan"))

    status = PASS if passed else FAIL
    print(f"{status}  n={n:6d}  RMS={rms:6.3f}m  P95={p95:6.3f}m  "
          f"PDOP={pdop:4.2f}  UERE={uere:5.3f}  sats={sats:.1f}  [{region}]")

    if not passed:
        for f in failures:
            print(f"       ↳ FAIL: {f}")

    if do_record:
        history.record(station, config_name, metrics)

    return {
        "status":  "pass" if passed else "fail",
        "metrics": metrics,
        "passed":  passed,
        "station": station,
        "config":  config_name,
        "region":  region,
    }


def load_existing_results() -> list:
    """Load all existing result CSVs from testing/results/ without re-running."""
    results = []
    if not os.path.exists(RESULTS_DIR):
        return results
    for fname in sorted(os.listdir(RESULTS_DIR)):
        if not fname.endswith(".csv"):
            continue
        # Filename: STATION_configname.csv
        # Find matching dataset and config
        csv_path = os.path.join(RESULTS_DIR, fname)
        stem = fname[:-4]  # strip .csv
        # Match against known datasets and configs
        matched_ds = None
        matched_cfg_name = None
        for ds in DATASETS:
            station = ds["station"]
            if stem.startswith(station + "_"):
                cfg_name = stem[len(station) + 1:]
                if cfg_name in CONFIGS:
                    matched_ds = ds
                    matched_cfg_name = cfg_name
                    break
        if matched_ds is None:
            continue
        obs_first = next(
            (_obs_file(matched_ds["station"], d)
             for d in _expand_dates(matched_ds["date"])
             if os.path.exists(_obs_file(matched_ds["station"], d))),
            None
        )
        truth = _truth_from_rinex(obs_first) if obs_first else None
        metrics = compute_metrics(csv_path, truth)
        cfg = CONFIGS[matched_cfg_name]
        region = matched_ds.get("region", "default")
        thresholds = resolve_thresholds(cfg, region)
        passed, _ = check_thresholds(metrics, thresholds)
        results.append({
            "status":  "pass" if passed else "fail",
            "metrics": metrics,
            "passed":  passed,
            "station": matched_ds["station"],
            "config":  matched_cfg_name,
            "region":  region,
        })
    return results


def main():
    parser = argparse.ArgumentParser(description="GNSS positioning test framework")
    parser.add_argument("--config",  action="append", help="Run only this config (repeatable)")
    parser.add_argument("--station", help="Run only this station")
    parser.add_argument("--record",  action="store_true", help="Save results to history")
    parser.add_argument("--history", action="store_true", help="Show historical trend and exit")
    parser.add_argument("--report",  action="store_true",
                        help="Generate PNG reports from existing results (no re-run)")
    args = parser.parse_args()

    if args.history:
        history.print_trend(config=args.config[0] if args.config else None,
                            station=args.station)
        return

    # --report alone: just generate from existing CSVs
    if args.report and not any([args.config, args.station, args.record]):
        print("Generating reports from existing results...")
        existing = load_existing_results()
        if not existing:
            print("No result CSVs found in testing/results/")
            return
        report.report_latest(existing)
        report.report_history(os.path.join(os.path.dirname(__file__), "history.csv"))
        return

    datasets = DATASETS
    configs  = CONFIGS

    if args.station:
        datasets = [d for d in datasets if d["station"] == args.station]
    if args.config:
        configs = {k: v for k, v in configs.items() if k in args.config}

    if not datasets:
        print(f"No station matching '{args.station}'"); sys.exit(1)
    if not configs:
        print(f"No config matching {args.config}"); sys.exit(1)

    total = passed = failed = skipped = 0
    all_results = []

    for cfg_name, cfg in configs.items():
        compatible = [d for d in datasets if dataset_supports_config(d, cfg)]
        if not compatible:
            continue
        print(f"\n[{cfg_name}]")
        for ds in compatible:
            result = run_one(ds, cfg_name, cfg, args.record)
            total += 1
            if result["status"] == "pass":
                passed  += 1
                all_results.append(result)
            elif result["status"] == "fail":
                failed  += 1
                all_results.append(result)
            else:
                skipped += 1

    print(f"\n{'='*65}")
    print(f"Results: {passed}/{total} passed  ({failed} failed, {skipped} skipped)")

    if args.record:
        print(f"Saved to: testing/history.csv")

    if args.report:
        print("\nGenerating reports...")
        # Merge run results with any existing CSVs not in this run
        existing = load_existing_results()
        # Existing results for configs not in this run
        run_keys = {(r["station"], r["config"]) for r in all_results}
        extra = [r for r in existing if (r["station"], r["config"]) not in run_keys]
        combined = all_results + extra
        report.report_latest(combined)
        report.report_history(
            os.path.join(os.path.dirname(__file__), "history.csv")
        )

    sys.exit(0 if failed == 0 else 1)


if __name__ == "__main__":
    main()
