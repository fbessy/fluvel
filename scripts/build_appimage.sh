#!/usr/bin/env bash
set -euo pipefail

# -------- Config --------
APP_NAME="Fluvel"
INSTALL_PREFIX="/usr"

# -------- Paths --------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

DIST_DIR="$PROJECT_ROOT/dist"
BUILD_DIR="$DIST_DIR/build-appimage"
APPDIR="$DIST_DIR/AppDir"

TOOLS_DIR="$PROJECT_ROOT/tools"
LINUXDEPLOY="$TOOLS_DIR/linuxdeploy-x86_64.AppImage"
QT_PLUGIN="$TOOLS_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"

cd "$PROJECT_ROOT"

echo "===> Cleaning dist directory"
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

# -------- Detect Qt --------
echo "===> Detecting Qt"

if command -v qmake6 >/dev/null 2>&1; then
  QMAKE=$(which qmake6)
elif command -v qmake >/dev/null 2>&1; then
  QMAKE=$(which qmake)
else
  echo "❌ Error: qmake not found"
  exit 1
fi

QT_ROOT="$(dirname "$(dirname "$QMAKE")")"
export Qt6_DIR="$QT_ROOT/lib/cmake/Qt6"
export CMAKE_PREFIX_PATH="$QT_ROOT/lib/cmake"

echo "QMAKE=$QMAKE"
echo "QT_ROOT=$QT_ROOT"
echo "Qt6_DIR=$Qt6_DIR"

# -------- Build --------
echo "===> Configuring (Release)"
cmake -S . -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX \
  -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH"

echo "===> Building"
cmake --build "$BUILD_DIR" -j$(nproc)

echo "===> Installing into AppDir"
cmake --install "$BUILD_DIR" --prefix "$APPDIR$INSTALL_PREFIX"

# -------- Desktop + icon --------
echo "===> Copying desktop and icon"
cp packaging/appimage/fluvel.desktop "$APPDIR/"
cp packaging/appimage/fluvel.png "$APPDIR/"

# -------- linuxdeploy --------
mkdir -p "$TOOLS_DIR"

if [ ! -f "$LINUXDEPLOY" ]; then
  echo "===> Downloading linuxdeploy..."
  wget -O "$LINUXDEPLOY" \
    https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
  chmod +x "$LINUXDEPLOY"
fi

if [ ! -f "$QT_PLUGIN" ]; then
  echo "===> Downloading linuxdeploy Qt plugin..."
  wget -O "$QT_PLUGIN" \
    https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
  chmod +x "$QT_PLUGIN"
fi

# -------- Filter Qt plugins (SAFE, no SDK touch) --------
echo "===> Filtering Qt plugins (removing TIFF safely)"

QT_PLUGINS="$QT_ROOT/plugins"
FILTERED_PLUGINS="$DIST_DIR/qt-plugins-filtered"

rm -rf "$FILTERED_PLUGINS"
mkdir -p "$FILTERED_PLUGINS"

rsync -a "$QT_PLUGINS/" "$FILTERED_PLUGINS/" \
  --exclude "imageformats/libqtiff.so*"

export QT_PLUGIN_PATH="$FILTERED_PLUGINS"

echo "===> Forcing Qt runtime environment"

export LD_LIBRARY_PATH="$QT_ROOT/lib"
export QT_PLUGIN_PATH="$QT_ROOT/plugins"
export QML2_IMPORT_PATH="$QT_ROOT/qml"

echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"

# -------- Build AppImage (ONE PASS ONLY) --------
echo "===> Building AppImage with linuxdeploy"

"$LINUXDEPLOY" \
  --appdir "$APPDIR" \
  --executable "$APPDIR/usr/bin/$APP_NAME" \
  --desktop-file "$APPDIR/fluvel.desktop" \
  --icon-file "$APPDIR/fluvel.png" \
  --plugin qt \
  --output appimage

# -------- Move & rename --------
echo "===> Moving AppImage"
mv ./*.AppImage "$DIST_DIR/"

VERSION=$(git describe --tags --always || echo "dev")
ARCH="x86_64"

mv "$DIST_DIR/"*.AppImage "$DIST_DIR/${APP_NAME}-${VERSION}-${ARCH}.AppImage"

# -------- Cleanup --------
echo "===> Cleaning temporary files"
rm -rf "$BUILD_DIR" "$APPDIR" "$FILTERED_PLUGINS"

echo "===> Done"
ls -lh "$DIST_DIR"/*.AppImage
