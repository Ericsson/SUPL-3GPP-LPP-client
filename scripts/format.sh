#!/bin/bash

if [[ "$1" = "--help" ]]; then
    echo "Usage: $0 [apply]"
    exit 0
fi

FAILED=0

for file in $(find . -name "*.cpp" -or -name "*.h" -or -name "*.hpp" -or -name "*.c" | grep -v -e "\.\/external" | grep -v -e "\/venv" | grep -v -e "\.\/build" | grep -v -e "\.\/docker" | grep -v -e "\.\/scripts" | grep -v -e "\.\/tests\/data" | grep -v -e "\.\/tokoro_verify")
do
    if [[ "$1" != "apply" ]]; then
        clang-format --Werror --style=file -n $file || FAILED=1
    else
        clang-format --style=file -i $file
    fi
done

exit $FAILED
