#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="build"
MANIFEST="fluvel.json"

flatpak-builder --install --user --force-clean "$BUILD_DIR" "$MANIFEST"
