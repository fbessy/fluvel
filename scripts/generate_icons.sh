#!/usr/bin/env bash
set -e

# =========================
# Resolve project root
# =========================
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# =========================
# Paths
# =========================
SRC="$ROOT_DIR/resources/icons/app/fluvel.svg"

ICON_DIR="$ROOT_DIR/resources/icons/app"
PACKAGING_DIR="$ROOT_DIR/packaging"

WIN_DIR="$PACKAGING_DIR/windows"
MAC_DIR="$PACKAGING_DIR/macos"
LINUX_DIR="$PACKAGING_DIR/linux"
ANDROID_RES="$PACKAGING_DIR/android/res"

DOC_DIR="$ROOT_DIR/docs/images"
PAGE_DIR="$ROOT_DIR/web"

TMP="$ROOT_DIR/.icon_tmp"

# =========================
# Check dependencies
# =========================
command -v rsvg-convert >/dev/null || {
    echo "Error: rsvg-convert not found"
    exit 1
}

command -v magick >/dev/null || {
    echo "Error: ImageMagick (magick) not found"
    exit 1
}

command -v png2icns >/dev/null || {
    echo "Error: png2icns not found"
    exit 1
}

# =========================
# Check source
# =========================
if [ ! -f "$SRC" ]; then
    echo "Error: SVG source not found: $SRC"
    exit 1
fi

echo "Project root: $ROOT_DIR"

# =========================
# Create directories
# =========================
mkdir -p "$ICON_DIR"
mkdir -p "$WIN_DIR"
mkdir -p "$MAC_DIR"
mkdir -p "$LINUX_DIR"

mkdir -p "$ANDROID_RES/mipmap-mdpi"
mkdir -p "$ANDROID_RES/mipmap-hdpi"
mkdir -p "$ANDROID_RES/mipmap-xhdpi"
mkdir -p "$ANDROID_RES/mipmap-xxhdpi"
mkdir -p "$ANDROID_RES/mipmap-xxxhdpi"

mkdir -p "$TMP"

# =========================
# Master PNG
# =========================
echo "Generating master PNG..."

rsvg-convert \
    -w 1024 \
    -h 1024 \
    "$SRC" > "$TMP/master.png"

# =========================
# App icons
# =========================
echo "Generating application PNG icons..."

rsvg-convert -w 16  -h 16  "$SRC" > "$ICON_DIR/fluvel-16.png"
rsvg-convert -w 22  -h 22  "$SRC" > "$ICON_DIR/fluvel-22.png"
rsvg-convert -w 32  -h 32  "$SRC" > "$ICON_DIR/fluvel-32.png"
rsvg-convert -w 48  -h 48  "$SRC" > "$ICON_DIR/fluvel-48.png"
rsvg-convert -w 64  -h 64  "$SRC" > "$ICON_DIR/fluvel-64.png"
rsvg-convert -w 128 -h 128 "$SRC" > "$ICON_DIR/fluvel-128.png"
rsvg-convert -w 256 -h 256 "$SRC" > "$ICON_DIR/fluvel-256.png"
rsvg-convert -w 512 -h 512 "$SRC" > "$ICON_DIR/fluvel-512.png"

# =========================
# Base sizes
# =========================
echo "Generating base PNG sizes..."

cp "$TMP/master.png" "$TMP/1024.png"

rsvg-convert -w 512 -h 512 "$SRC" > "$TMP/512.png"
rsvg-convert -w 256 -h 256 "$SRC" > "$TMP/256.png"
rsvg-convert -w 128 -h 128 "$SRC" > "$TMP/128.png"

# =========================
# Windows ICO
# =========================
echo "Generating Windows ICO..."

magick "$TMP/master.png" \
    -define icon:auto-resize=256,128,64,48,32,16 \
    "$WIN_DIR/Fluvel.ico"

cp "$WIN_DIR/Fluvel.ico" "$ROOT_DIR/resources/platforms/windows/Fluvel.ico"

# =========================
# macOS ICNS
# =========================
echo "Generating macOS ICNS..."

png2icns "$MAC_DIR/Fluvel.icns" \
    "$TMP/1024.png" \
    "$TMP/512.png" \
    "$TMP/256.png" \
    "$TMP/128.png"

# =========================
# Linux / Flatpak
# =========================
echo "Generating Flatpak/Linux icon..."

cp "$SRC" "$LINUX_DIR/org.fluvel.App.svg"

# =========================
# Linux / AppImage
# =========================
echo "Generating AppImage icon..."

cp "$ICON_DIR/fluvel-256.png" \
   "$PACKAGING_DIR/appimage/fluvel.png"

# =========================
# Android
# =========================
echo "Generating Android icons..."

rsvg-convert -w 48  -h 48  "$SRC" > "$ANDROID_RES/mipmap-mdpi/fluvel.png"
rsvg-convert -w 72  -h 72  "$SRC" > "$ANDROID_RES/mipmap-hdpi/fluvel.png"
rsvg-convert -w 96  -h 96  "$SRC" > "$ANDROID_RES/mipmap-xhdpi/fluvel.png"
rsvg-convert -w 144 -h 144 "$SRC" > "$ANDROID_RES/mipmap-xxhdpi/fluvel.png"
rsvg-convert -w 192 -h 192 "$SRC" > "$ANDROID_RES/mipmap-xxxhdpi/fluvel.png"

# =======================================
# Icon for homage page and documentation
# =======================================
cp "$SRC" \
   "$DOC_DIR/fluvel.svg"

cp "$SRC" \
   "$PAGE_DIR/fluvel.svg"

# =========================
# Cleanup
# =========================
echo "Cleaning temporary files..."
rm -rf "$TMP"

echo ""
echo "Icons successfully generated."
