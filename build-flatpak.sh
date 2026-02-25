#!/usr/bin/env bash
set -e

BUILD_DIR="build"
MANIFEST="ofeli.json"

flatpak-builder --install --user --force-clean "$BUILD_DIR" "$MANIFEST"
