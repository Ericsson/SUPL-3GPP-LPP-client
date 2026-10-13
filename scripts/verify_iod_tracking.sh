#!/bin/bash

# Verification script for IOD tracking functionality

echo "=== IOD Tracking Verification ==="
echo ""

# Check if the binary exists
if [ ! -f "build/example-client" ]; then
    echo "ERROR: example-client binary not found"
    echo "Please build the project first: cd build && ninja example-client"
    exit 1
fi

echo "✓ Binary found: build/example-client"
echo ""

# Check if IOD_DEBUG_TRACKING is enabled in the binary
if strings build/example-client | grep -q "iod_ssr_orbit.csv"; then
    echo "✓ IOD_DEBUG_TRACKING is enabled in the binary"
else
    echo "✗ IOD_DEBUG_TRACKING not found in binary"
    echo "  Make sure IOD_DEBUG_TRACKING=1 is set in CMakeLists.txt"
    exit 1
fi

echo ""
echo "=== Modified Files ==="
echo "The following files have been modified to add IOD tracking:"
echo ""
echo "1. dependency/generator/tokoro/data/gather.cpp"
echo "   - Added SSR orbit correction IOD tracking"
echo ""
echo "2. examples/client/processor/lpp2eph.cpp"
echo "   - Added LPP ephemeris IOD tracking (GPS, Galileo, BeiDou)"
echo ""
echo "3. examples/client/processor/rtcm2eph.cpp"
echo "   - Added RTCM ephemeris IOD tracking (GPS, Galileo, BeiDou)"
echo ""
echo "4. examples/client/CMakeLists.txt"
echo "   - Added IOD_DEBUG_TRACKING compile definition"
echo ""
echo "5. dependency/generator/tokoro/CMakeLists.txt"
echo "   - Added IOD_DEBUG_TRACKING compile definition"
echo ""

echo "=== Output Files ==="
echo "When you run example-client, the following CSV files will be created:"
echo ""
echo "1. iod_ssr_orbit.csv"
echo "   Columns: gnss,prn,ssr_iod,iod,epoch_tai,current_tai"
echo ""
echo "2. iod_ephemeris.csv"
echo "   Columns: gnss,prn,lpp_iod,iode,iodc,toe,current_tai"
echo ""

echo "=== GNSS ID Reference ==="
echo "0 = GPS"
echo "2 = Galileo"
echo "3 = BeiDou"
echo ""

echo "=== Usage ==="
echo "Run example-client as normal. The CSV files will be created in the"
echo "working directory and appended to on each run."
echo ""
echo "Example:"
echo "  cd build"
echo "  ./example-client [your arguments]"
echo ""
echo "Then analyze the CSV files with your preferred tool (Excel, Python, R, etc.)"
echo ""

echo "=== Documentation ==="
echo "See IOD_TRACKING_README.md for detailed usage instructions"
echo "See IOD_TRACKING_SUMMARY.md for implementation details"
echo ""

echo "✓ Verification complete!"
