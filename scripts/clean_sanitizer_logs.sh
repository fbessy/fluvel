#!/usr/bin/env bash
# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Cleaning sanitizer logs in: ${SCRIPT_DIR}"

find "${SCRIPT_DIR}" \
    -maxdepth 1 \
    -type f \
    \( -name 'asan_log*' -o -name 'tsan_log*' \) \
    -print \
    -delete

echo "Done."
