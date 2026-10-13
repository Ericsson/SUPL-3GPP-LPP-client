#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
TEST_DATA_DIR="$PROJECT_DIR/tests/data/rtcm/captured"

CLIENT="$BUILD_DIR/example-client"

if [ ! -f "$CLIENT" ]; then
    echo "Error: example-client not found at $CLIENT"
    echo "Please build the project first: ninja -C build"
    exit 1
fi

if [ $# -lt 2 ]; then
    echo "Usage: $0 <location-server-host> <test-case-name> [additional-args...]"
    echo ""
    echo "Test cases:"
    echo "  default          - All GNSS, auto MSM"
    echo "  gps_only         - GPS only"
    echo "  msm4             - Force MSM4"
    echo "  msm7             - Force MSM7"
    echo "  multi_gnss       - All GNSS systems"
    echo "  with_residuals   - Include residuals"
    echo "  with_bias        - Include bias information"
    echo ""
    echo "Example:"
    echo "  $0 location-server.com default"
    echo "  $0 location-server.com gps_only --ls-port=5431"
    exit 1
fi

LS_HOST="$1"
TEST_CASE="$2"
shift 2

mkdir -p "$TEST_DATA_DIR"

TIMESTAMP=$(date +%Y%m%d_%H%M%S)
UPER_FILE="$TEST_DATA_DIR/${TEST_CASE}-${TIMESTAMP}.uper"
RTCM_FILE="$TEST_DATA_DIR/${TEST_CASE}-${TIMESTAMP}.rtcm"

TEMP_UPER="$TEST_DATA_DIR/.${TEST_CASE}-${TIMESTAMP}.uper.tmp"
TEMP_RTCM="$TEST_DATA_DIR/.${TEST_CASE}-${TIMESTAMP}.rtcm.tmp"

rm -f "$TEMP_UPER" "$TEMP_RTCM"

BASE_ARGS=(
    --ls-host="$LS_HOST"
    --ls-port=5431
    --ipv4 1.1.1.1
    --ad-type=osr
    --ad-osr-observations=1
    --lpp2rtcm
    --mcc 240 --mnc 1 --tac 1 --ci 1
    --l2r-shutdown-after=1
    --output "file:path=$TEMP_UPER,format=lpp-uper"
    --output "file:path=$TEMP_RTCM,format=rtcm"
)

case "$TEST_CASE" in
    default)
        EXTRA_ARGS=()
        ;;
    gps_only)
        EXTRA_ARGS=(
            --ad-no-glo --ad-no-gal --ad-no-bds
            --l2r-no-glo --l2r-no-gal --l2r-no-bds
        )
        ;;
    msm4)
        EXTRA_ARGS=(--l2r-msm=4)
        ;;
    msm7)
        EXTRA_ARGS=(--l2r-msm=7)
        ;;
    *)
        echo "Error: Unknown test case '$TEST_CASE'"
        exit 1
        ;;
esac

echo "Capturing test data for: $TEST_CASE"
echo "Location server: $LS_HOST"
echo "Output files:"
echo "  UPER: $UPER_FILE"
echo "  RTCM: $RTCM_FILE"
echo ""
echo "Starting capture (will stop automatically after 1 conversion)..."
echo ""
echo "$CLIENT" "${BASE_ARGS[@]}" "${EXTRA_ARGS[@]}" "$@"

"$CLIENT" "${BASE_ARGS[@]}" "${EXTRA_ARGS[@]}" "$@"

if [ ! -f "$TEMP_UPER" ] || [ ! -s "$TEMP_UPER" ]; then
    echo "Error: No UPER data captured"
    rm -f "$TEMP_UPER" "$TEMP_RTCM"
    exit 1
fi

if [ ! -f "$TEMP_RTCM" ] || [ ! -s "$TEMP_RTCM" ]; then
    echo "Error: No RTCM data captured"
    rm -f "$TEMP_UPER" "$TEMP_RTCM"
    exit 1
fi

UPER_SIZE=$(stat -f%z "$TEMP_UPER" 2>/dev/null || stat -c%s "$TEMP_UPER" 2>/dev/null)
RTCM_SIZE=$(stat -f%z "$TEMP_RTCM" 2>/dev/null || stat -c%s "$TEMP_RTCM" 2>/dev/null)

echo ""
echo "Capture successful!"
echo "  UPER: $UPER_SIZE bytes"
echo "  RTCM: $RTCM_SIZE bytes"

mv "$TEMP_UPER" "$UPER_FILE"
mv "$TEMP_RTCM" "$RTCM_FILE"

echo ""
echo "Files saved:"
echo "  $UPER_FILE"
echo "  $RTCM_FILE"
