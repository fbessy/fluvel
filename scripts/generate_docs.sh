#!/usr/bin/env bash

set -e

# Root du projet
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT"

OUTPUT_DIR="docs/html"

echo "🧹 Cleaning previous documentation..."

# Sécurité : éviter rm sur chemin vide
if [[ -n "$OUTPUT_DIR" && -d "$OUTPUT_DIR" ]]; then
    rm -rf "$OUTPUT_DIR"
fi

echo "📘 Generating Doxygen documentation..."

doxygen Doxyfile

echo "🌐 Opening documentation..."

xdg-open docs/html/index.html > /dev/null 2>&1 || true

echo "✅ Done"
