#!/bin/bash
# Reproduce fuzzer findings to identify issues

FUZZER=$1
CORPUS=$2

if [ -z "$FUZZER" ] || [ -z "$CORPUS" ]; then
    echo "Usage: $0 <fuzzer_binary> <corpus_dir>"
    echo "Example: $0 ./tests/fuzz_ubx ../tests/corpus/ubx"
    exit 1
fi

echo "Testing all corpus files with $FUZZER..."
for file in "$CORPUS"/*; do
    if [ -f "$file" ]; then
        echo "Testing: $(basename $file)"
        timeout 1 "$FUZZER" "$file" 2>&1 | grep -E "(ERROR|ASAN|leak|crash)" && echo "  ^ Issue found!"
    fi
done

echo ""
echo "To debug a specific file:"
echo "  gdb --args $FUZZER <corpus_file>"
echo ""
echo "To minimize a crashing input:"
echo "  $FUZZER -minimize_crash=1 <crash_file>"
