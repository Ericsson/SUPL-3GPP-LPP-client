#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ $# -lt 1 ]; then
    echo "Usage: $0 <location-server-host> [additional-args...]"
    echo ""
    echo "This script captures all RTCM test cases."
    echo ""
    echo "Example:"
    echo "  $0 location-server.com"
    echo "  $0 location-server.com --ls-port=5431"
    exit 1
fi

LS_HOST="$1"
shift

TEST_CASES=(
    "default"
    "gps_only"
    "msm4"
    "msm7"
    "multi_gnss"
    "with_residuals"
    "with_bias"
)

echo "Capturing all RTCM test cases from $LS_HOST"
echo ""

FAILED=()

for test_case in "${TEST_CASES[@]}"; do
    echo "=========================================="
    echo "Capturing: $test_case"
    echo "=========================================="
    
    if "$SCRIPT_DIR/capture_rtcm_test_data.sh" "$LS_HOST" "$test_case" "$@"; then
        echo "✓ $test_case captured successfully"
    else
        echo "✗ $test_case failed"
        FAILED+=("$test_case")
    fi
    
    echo ""
    sleep 2
done

echo "=========================================="
echo "Summary"
echo "=========================================="
echo "Total test cases: ${#TEST_CASES[@]}"
echo "Successful: $((${#TEST_CASES[@]} - ${#FAILED[@]}))"
echo "Failed: ${#FAILED[@]}"

if [ ${#FAILED[@]} -gt 0 ]; then
    echo ""
    echo "Failed test cases:"
    for test_case in "${FAILED[@]}"; do
        echo "  - $test_case"
    done
    exit 1
fi

echo ""
echo "All test cases captured successfully!"
echo "Run tests with: ninja -C build test"
