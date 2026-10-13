#!/bin/bash
set -e

BUILD_DIR="${1:-build}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "Error: compile_commands.json not found in $BUILD_DIR"
    exit 1
fi

IWYU_TOOL=$(which iwyu_tool.py || which iwyu_tool || echo "")
if [ -n "$IWYU_TOOL" ]; then
    echo "Running include-what-you-use with iwyu_tool..."
    $IWYU_TOOL -p "$BUILD_DIR" \
        -- -Xiwyu --mapping_file="$PROJECT_DIR/cmake/iwyu.imp" \
        -Xiwyu --transitive_includes_only
else
    echo "Error: iwyu_tool.py not found"
    echo "Install with: pip install iwyu or use your package manager"
    exit 1
fi

echo "IWYU analysis complete!"
