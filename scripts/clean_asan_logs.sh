#!/usr/bin/env bash
# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Cleaning ASan logs in: $SCRIPT_DIR"

# Supprime uniquement les fichiers asan_log.*
find "$SCRIPT_DIR" -type f -name "asan_log.*" -print -delete

echo "Done."
