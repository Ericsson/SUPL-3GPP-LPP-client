#!/bin/bash
set -e

BUILD_DIR="build"
CHECKS=""
JOBS="${JOBS:-$(nproc)}"
QUIET=0
COMPACT=0
FIX=0

while [[ $# -gt 0 ]]; do
    case $1 in
        --quiet)
            QUIET=1
            shift
            ;;
        --compact)
            COMPACT=1
            shift
            ;;
        --fix)
            FIX=1
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        --build)
            BUILD_DIR="$2"
            shift 2
            ;;
        --checks)
            CHECKS="$2"
            shift 2
            ;;
        -*)
            echo "Unknown option: $1"
            exit 1
            ;;
        *)
            echo "Unknown argument: $1"
            exit 1
            ;;
    esac
done

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "Error: compile_commands.json not found in $BUILD_DIR"
    exit 1
fi

LOG_DIR="$BUILD_DIR/clang-tidy-log"
mkdir -p "$LOG_DIR"

run_clang_tidy() {
    local file="$1"
    local log_file="$LOG_DIR/$(echo "$file" | sed 's/\//_/g').log"
    
    local fix_flags=""
    if [ "$FIX" -eq 1 ]; then
        fix_flags="--fix --fix-errors"
    fi
    
    if [ -n "$CHECKS" ]; then
        clang-tidy -p "$BUILD_DIR" --checks="-*,$CHECKS" --quiet --use-color $fix_flags "$file" 2>&1 | grep -v "^Suppressed" | grep -v "^Use -header-filter" | grep -v "^[0-9]* warnings" > "$log_file" || true
    else
        clang-tidy -p "$BUILD_DIR" --quiet --use-color $fix_flags "$file" 2>&1 | grep -v "^Suppressed" | grep -v "^Use -header-filter" | grep -v "^[0-9]* warnings" > "$log_file" || true
    fi
    
    if [ -s "$log_file" ] && grep -q "error:\|warning:" "$log_file"; then
        if [ "$COMPACT" -eq 1 ]; then
            grep "error:\|warning:" "$log_file"
        else
            echo -e "\033[31m[ERR]\033[0m $file"
            cat "$log_file"
        fi
        return 1
    else
        if [ "$QUIET" -eq 0 ]; then
            echo -e "\033[32m[ OK]\033[0m $file" 
        fi
        rm -f "$log_file"
        return 0
    fi
}

export -f run_clang_tidy
export BUILD_DIR
export CHECKS
export LOG_DIR
export QUIET
export COMPACT
export FIX

if [ -n "$CHECKS" ]; then
    echo "Running clang-tidy with checks: $CHECKS (jobs: $JOBS)"
else
    echo "Running clang-tidy... (jobs: $JOBS)"
fi

if [ "$QUIET" -eq 1 ]; then
    echo "Quiet mode: only showing failures"
fi

if [ "$COMPACT" -eq 1 ]; then
    echo "Compact mode: only showing error/warning lines"
fi

find dependency examples -name '*.cpp' -not -path '*/external/*' -not -path './tests/*' -not -path '*/idokeido/*' | \
    xargs -P"$JOBS" -I{} bash -c 'run_clang_tidy "{}"' || true

echo "Analysis complete! Logs saved in $LOG_DIR"
