#!/usr/bin/env bash
# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

set -euo pipefail

# =========================================================
# Resolve project root
# =========================================================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# =========================================================
# Paths
# =========================================================
VIDEO_DIR="$ROOT_DIR/assets/videos"
GIF_DIR="$ROOT_DIR/assets/gifs"

mkdir -p "$GIF_DIR"

# =========================================================
# Check dependencies
# =========================================================
command -v ffmpeg >/dev/null || {
    echo "Error: ffmpeg not found"
    exit 1
}

# =========================================================
# Generate GIF helper
# =========================================================
generate_gif()
{
    local input="$1"
    local output="$2"

    echo "Generating $(basename "$output")..."

    ffmpeg -y \
        -i "$input" \
        -vf "fps=10,scale=960:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" \
        "$output"
}

# =========================================================
# Generate GIFs
# =========================================================
generate_gif \
    "$VIDEO_DIR/mri_segmentation.webm" \
    "$GIF_DIR/mri_segmentation.gif"

generate_gif \
    "$VIDEO_DIR/phone_tracking.webm" \
    "$GIF_DIR/phone_tracking.gif"

echo ""
echo "GIFs successfully generated."
