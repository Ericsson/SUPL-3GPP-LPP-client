# GNSS Positioning Test Framework

Automated accuracy characterization for SPP and PPP engines.

## Usage

```bash
# Run all tests (downloads data if needed)
python3 testing/run_all.py

# Run specific config only
python3 testing/run_all.py --config spp_gps

# Run specific station only
python3 testing/run_all.py --station MAR600SWE

# Record results to history
python3 testing/run_all.py --record

# Show history trend
python3 testing/run_all.py --history
```

## Adding a new configuration

Edit `testing/configs.py` — it will automatically run against all stations.

## Adding a new station

Edit `testing/datasets.py` — it will automatically run all configurations.

## Output

- Console: pass/fail table with key metrics
- `testing/results/<station>_<config>.csv`: per-epoch data
- `testing/history.csv`: historical results per commit
