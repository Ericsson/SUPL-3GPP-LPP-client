#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTDIR="$SCRIPT_DIR"

WEEK="2393"
YEAR="2025"
DAY="320"
YY="25"
DOY="320"
STATION="CHOF00JPN"

echo "Downloading satellite data for ${YEAR}-${DAY}..."

# Download SP3 precise orbits
if [ -f "$OUTDIR/igs.sp3" ]; then
  echo "SP3 file exists, skipping"
else
  echo "Fetching SP3..."
  wget --auth-no-challenge -q --show-progress \
    -O "$OUTDIR/igs.sp3.Z" \
    "https://cddis.nasa.gov/archive/gnss/products/${WEEK}/IGS0DEMULT_${YEAR}${DAY}0000_02D_05M_ORB.SP3.gz" || \
    echo "SP3 download failed"

  [ -f "$OUTDIR/igs.sp3.Z" ] && uncompress "$OUTDIR/igs.sp3.Z"
fi

# Download station observation file
if [ -f "$OUTDIR/${STATION}.crx" ]; then
  echo "Station file exists, skipping"
else
  echo "Fetching station observations..."
  wget --auth-no-challenge -q --show-progress \
    -O "$OUTDIR/${STATION}.crx.gz" \
    "https://cddis.nasa.gov/archive/gnss/data/daily/${YEAR}/${DOY}/${YY}d/${STATION}_S_${YEAR}${DAY}0000_01D_30S_MO.crx.gz" || \
    echo "Station download failed"

  [ -f "$OUTDIR/${STATION}.crx.gz" ] && gunzip -f "$OUTDIR/${STATION}.crx.gz"
fi

# Download broadcast navigation ephemeris
if [ -f "$OUTDIR/brdc_nav.rnx" ]; then
  echo "Broadcast nav exists, skipping"
else
  echo "Fetching broadcast ephemeris..."
  wget --auth-no-challenge -q --show-progress \
    -O "$OUTDIR/brdc_nav.rnx.gz" \
    "https://cddis.nasa.gov/archive/gnss/data/daily/${YEAR}/brdc/BRDC00IGS_R_${YEAR}${DAY}0000_01D_MN.rnx.gz" || \
    echo "Broadcast nav download failed"

  [ -f "$OUTDIR/brdc_nav.rnx.gz" ] && gunzip -f "$OUTDIR/brdc_nav.rnx.gz"
fi

# Download multi-gnss broadcast navigation ephemeris
if [ -f "$OUTDIR/brdc_nav_multi.rnx" ]; then
  echo "Multi-gnss broadcast nav exists, skipping"
else
  echo "Fetching multi-gnss broadcast ephemeris..."
  wget --auth-no-challenge -q --show-progress \
    -O "$OUTDIR/brdc_nav_multi.rnx.gz" \
    "https://cddis.nasa.gov/archive/gnss/data/daily/${YEAR}/brdc/BRDM00DLR_S_${YEAR}${DAY}0000_01D_MN.rnx.gz" || \
    echo "Multi-gnss broadcast nav download failed"

  [ -f "$OUTDIR/brdc_nav_multi.rnx.gz" ] && gunzip -f "$OUTDIR/brdc_nav_multi.rnx.gz"
fi

echo "Done!"
ls -lh "$OUTDIR/"
