#!/usr/bin/env bash
# update_translations.sh

set -e

# =========================
# Resolve project root
# =========================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# =========================
# Qt tools path
# =========================
QT_BIN="$HOME/Qt/6.10.2/gcc_64/bin"

# =========================
# Source directories
# =========================
SRC_DIRS=("src/app" "src/image_processing")

# =========================
# Translation directory
# =========================
TS_DIR="$ROOT_DIR/translations"
mkdir -p "$TS_DIR"

# =========================
# Build source paths
# =========================
SRC_PATHS=()
for dir in "${SRC_DIRS[@]}"; do
    SRC_PATHS+=("$ROOT_DIR/$dir")
done

# =========================
# Process .ts files
# =========================
for ts_file in "$TS_DIR"/*.ts; do
    [ -e "$ts_file" ] || continue

    qm_file="${ts_file%.ts}.qm"

    echo "➡️  Updating $ts_file"
    "$QT_BIN/lupdate" "${SRC_PATHS[@]}" -ts "$ts_file"

    echo "➡️  Compiling $qm_file"
    "$QT_BIN/lrelease" "$ts_file"
done

echo "✅ All translations updated"
