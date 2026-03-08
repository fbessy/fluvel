#!/usr/bin/env bash
set -e

SRC="resources/icons/app/fluvel.svg"

ICON_DIR="resources/icons/app"
PLATFORM_DIR="resources/platforms"

WIN_DIR="$PLATFORM_DIR/windows"
MAC_DIR="$PLATFORM_DIR/macos"
LINUX_DIR="$PLATFORM_DIR/linux"
ANDROID_RES="$PLATFORM_DIR/android/res"

TMP=".icon_tmp"

if [ ! -f "$SRC" ]; then
    echo "Error: SVG source not found: $SRC"
    exit 1
fi

echo "Creating directories..."

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

echo "Generating PNG sizes for application..."

magick "$SRC" -filter point -resize 16x16  "$ICON_DIR/fluvel-16.png"
magick "$SRC" -resize 22x22 "$ICON_DIR/fluvel-22.png"
magick "$SRC" -resize 32x32 "$ICON_DIR/fluvel-32.png"
magick "$SRC" -resize 48x48 "$ICON_DIR/fluvel-48.png"
magick "$SRC" -resize 64x64 "$ICON_DIR/fluvel-64.png"
magick "$SRC" -resize 128x128 "$ICON_DIR/fluvel-128.png"
magick "$SRC" -resize 256x256 "$ICON_DIR/fluvel-256.png"

echo "Generating base PNG sizes..."

magick "$SRC" -resize 1024x1024 "$TMP/1024.png"
magick "$SRC" -resize 512x512  "$TMP/512.png"
magick "$SRC" -resize 256x256  "$TMP/256.png"
magick "$SRC" -resize 128x128  "$TMP/128.png"

echo "Generating Windows ICO..."

magick "$SRC" -define icon:auto-resize=256,128,64,48,32,16 \
"$WIN_DIR/fluvel.ico"

echo "Generating macOS ICNS..."

png2icns "$MAC_DIR/Fluvel.icns" \
"$TMP/1024.png" \
"$TMP/512.png" \
"$TMP/256.png" \
"$TMP/128.png"

echo "Generating Flatpak/Linux icon..."

cp "$SRC" "$LINUX_DIR/org.fluvel.App.svg"

echo "Generating Android icons..."

magick "$SRC" -resize 48x48   "$ANDROID_RES/mipmap-mdpi/fluvel.png"
magick "$SRC" -resize 72x72   "$ANDROID_RES/mipmap-hdpi/fluvel.png"
magick "$SRC" -resize 96x96   "$ANDROID_RES/mipmap-xhdpi/fluvel.png"
magick "$SRC" -resize 144x144 "$ANDROID_RES/mipmap-xxhdpi/fluvel.png"
magick "$SRC" -resize 192x192 "$ANDROID_RES/mipmap-xxxhdpi/fluvel.png"

echo "Cleaning temporary files..."
rm -rf "$TMP"

echo ""
echo "Icons successfully generated."
