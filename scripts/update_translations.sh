#!/usr/bin/env bash
# update_translations.sh
# Update TS files and generate QM files

set -euo pipefail

# =========================
# Resolve project root
# =========================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "📁 Project root: $ROOT_DIR"

# =========================
# Find Qt tools
# =========================
LUPDATE=$(command -v lupdate || true)
LRELEASE=$(command -v lrelease || true)

if [ -z "$LUPDATE" ] || [ -z "$LRELEASE" ]; then
    echo "❌ Qt tools not found in PATH"
    echo "👉 You can fix it with:"
    echo "   export PATH=\$HOME/Qt/6.11.0/gcc_64/bin:\$PATH"
    exit 1
fi

# =========================
# Source directories
# =========================
SRC_DIRS=("src/app" "src/image_processing")

SRC_PATHS=()
for dir in "${SRC_DIRS[@]}"; do
    SRC_PATHS+=("$ROOT_DIR/$dir")
done

# =========================
# Translation directory
# =========================
TS_DIR="$ROOT_DIR/translations"
mkdir -p "$TS_DIR"

# =========================
# Languages (edit here)
# =========================
TS_FILES=(
    "fluvel_fr.ts"
    # "fluvel_en.ts"
    # "fluvel_es.ts"
)

# =========================
# Process translations
# =========================
for ts in "${TS_FILES[@]}"; do
    ts_path="$TS_DIR/$ts"
    qm_path="${ts_path%.ts}.qm"

    # Create if missing
    if [ ! -f "$ts_path" ]; then
        echo "🆕 Creating $ts"
        "$LUPDATE" "${SRC_PATHS[@]}" -ts "$ts_path"
    fi

    # Update
    echo "🔄 Updating $ts"
    "$LUPDATE" "${SRC_PATHS[@]}" -no-obsolete -ts "$ts_path"

    # Compile
    echo "⚙️  Compiling $(basename "$qm_path")"
    "$LRELEASE" "$ts_path"
done

echo "✅ Translations updated successfully"
