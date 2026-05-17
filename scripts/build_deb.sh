#!/usr/bin/env bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="build-deb"

# --- Clean previous build ---
rm -rf "${PROJECT_ROOT}/${BUILD_DIR}"

# --- Configure ---
cmake -S "${PROJECT_ROOT}" \
      -B "${PROJECT_ROOT}/${BUILD_DIR}" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH="$HOME/Qt/6.11.0/gcc_64"

# --- Build ---
cmake --build "${PROJECT_ROOT}/${BUILD_DIR}" -j

# --- Package ---
cd "${PROJECT_ROOT}/${BUILD_DIR}"

cpack
