#!/usr/bin/env bash
# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

set -euo pipefail

#######################################
# PATHS
#######################################

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="build-clang-tidy"
QT_PATH="$HOME/Qt/6.11.0/gcc_64"

#######################################
# CLEAN (idempotent)
#######################################

echo "🧹 Cleaning ${PROJECT_ROOT}/${BUILD_DIR}..."
rm -rf "${PROJECT_ROOT}/${BUILD_DIR}"

#######################################
# CONFIGURE
#######################################

cmake \
    -S "${PROJECT_ROOT}" \
    -B "${PROJECT_ROOT}/${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_PREFIX_PATH="${QT_PATH}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DFLUVEL_BUILD_APP=ON \
    -DFLUVEL_BUILD_BIND=ON

#######################################
# RUN CLANG-TIDY
#######################################

echo "Running clang-tidy..."

find "${PROJECT_ROOT}/src" \
    \( -name "*.cpp" -o -name "*.hpp" \) \
    -print0 |
while IFS= read -r -d '' file
do
    echo "Checking ${file}"

    clang-tidy \
        -p "${PROJECT_ROOT}/${BUILD_DIR}" \
        "${file}" \
        > fluvel_tidy_log.txt 2>&1
done

echo "Done."
