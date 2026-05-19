#!/usr/bin/env bash
# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

set -euo pipefail

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
