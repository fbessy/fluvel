#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

BUILD_DIR="$PROJECT_ROOT/build-flatpak"
MANIFEST="$PROJECT_ROOT/packaging/flatpak/fluvel.json"

flatpak-builder --install --user --force-clean "$BUILD_DIR" "$MANIFEST"
