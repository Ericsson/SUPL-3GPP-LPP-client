#!/bin/bash
# Download IGS RINEX data for SPP/PPP validation from BKG archive (no auth required).
#
# Usage: ./scripts/download_igs.sh [station] [date_or_range] [output_dir]
#   station        : 9-char IGS station name (default: TSKB00JPN)
#   date_or_range  : YYYY-MM-DD or YYYY-MM-DD:YYYY-MM-DD for a range (default: yesterday)
#   output_dir     : where to save files (default: ./igs_data)
#
# Examples:
#   ./scripts/download_igs.sh MAR600SWE 2026-03-25
#   ./scripts/download_igs.sh MAR600SWE 2026-03-25:2026-03-27
#
# Requires: curl, gunzip, crx2rnx

set -uo pipefail

STATION=${1:-"TSKB00JPN"}
DATE_ARG=${2:-$(date -u -d "yesterday" +%Y-%m-%d 2>/dev/null || date -u -v-1d +%Y-%m-%d)}
OUTDIR=${3:-"./igs_data"}

# Expand date range using Python
if [[ "$DATE_ARG" == *":"* ]]; then
    START_DATE=$(echo "$DATE_ARG" | cut -d: -f1)
    END_DATE=$(echo "$DATE_ARG" | cut -d: -f2)
    mapfile -t DATES < <(python3 -c "
import datetime
d = datetime.date.fromisoformat('$START_DATE')
e = datetime.date.fromisoformat('$END_DATE')
while d <= e:
    print(d)
    d += datetime.timedelta(days=1)
")
else
    DATES=("$DATE_ARG")
fi

download_one() {
local DATE="$1"
YEAR=$(echo "$DATE" | cut -d- -f1)
MONTH=$(echo "$DATE" | cut -d- -f2)
DAY=$(echo "$DATE" | cut -d- -f3)
DOY=$(date -d "$DATE" +%j 2>/dev/null || python3 -c "import datetime; d=datetime.date($YEAR,$MONTH,$DAY); print(d.timetuple().tm_yday)")
DOY=$(printf "%03d" "$((10#$DOY))")

STATION_UPPER=$(echo "$STATION" | tr '[:lower:]' '[:upper:]')
BASE="https://igs.bkg.bund.de/root_ftp/IGS"

OBS_FILE="${STATION_UPPER}_R_${YEAR}${DOY}0000_01D_30S_MO.crx.gz"
NAV_FILE="BRDM00DLR_S_${YEAR}${DOY}0000_01D_MN.rnx.gz"

OBS_URL="${BASE}/obs/${YEAR}/${DOY}/${OBS_FILE}"
NAV_URL="${BASE}/BRDC/${YEAR}/${DOY}/${NAV_FILE}"

mkdir -p "$OUTDIR"

echo "Station : $STATION_UPPER"
echo "Date    : $DATE (DOY $DOY)"
echo "Output  : $OUTDIR"
echo ""

echo "Downloading obs..."
curl -fsSL --progress-bar -o "$OUTDIR/$OBS_FILE" "$OBS_URL"

echo "Downloading nav..."
curl -fsSL --progress-bar -o "$OUTDIR/$NAV_FILE" "$NAV_URL"

# Decompress nav (plain gzip -> .rnx)
gunzip -f "$OUTDIR/$NAV_FILE"
NAV_RNX="$OUTDIR/${NAV_FILE%.gz}"

# Decompress obs (gzip -> .crx, then Hatanaka -> .rnx)
gunzip -f "$OUTDIR/$OBS_FILE"
CRX_FILE="$OUTDIR/${OBS_FILE%.gz}"

if command -v crx2rnx &>/dev/null; then
    OBS_RNX="${CRX_FILE%.crx}.rnx"
    crx2rnx -o "$OBS_RNX" "$CRX_FILE"
    rm "$CRX_FILE"
else
    echo ""
    echo "ERROR: crx2rnx not found. The obs file is in Hatanaka format (.crx)."
    echo "  Install: sudo apt install rnxcmp"
    echo "  Or download from: https://terras.gsi.go.jp/ja/crx2rnx.html"
    exit 1
fi

# Extract truth position directly from RINEX obs header
TRUTH_XYZ=$(grep "APPROX POSITION XYZ" "$OBS_RNX" | awk '{print $1, $2, $3}')

echo ""
echo "Files ready:"
echo "  Nav: $NAV_RNX"
echo "  Obs: $OBS_RNX"
echo ""

if [ -n "$TRUTH_XYZ" ]; then
    echo "Run SPP:"
    echo "  ./build/examples/spp/example_spp $NAV_RNX $OBS_RNX $TRUTH_XYZ"
else
    echo "Run SPP (no truth):"
    echo "  ./build/examples/spp/example_spp $NAV_RNX $OBS_RNX"
fi
} # end download_one

for DATE in "${DATES[@]}"; do
    download_one "$DATE" || echo "WARNING: failed to download $DATE, skipping"
done
