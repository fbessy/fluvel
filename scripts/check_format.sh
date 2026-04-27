#!/bin/bash

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

FILES=$(find "$PROJECT_ROOT/src" \( -name "*.cpp" -o -name "*.hpp" \))

FAILED=0

for file in $FILES; do
    if ! clang-format --dry-run --Werror "$file"; then
        echo "Format issue in $file"
        FAILED=1
    fi
done

exit $FAILED
