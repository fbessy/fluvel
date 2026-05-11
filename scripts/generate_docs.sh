#!/usr/bin/env bash

set -Eeuo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

SPHINX_SOURCE="$PROJECT_ROOT/docs/sphinx/source"
SPHINX_BUILD="$PROJECT_ROOT/docs/sphinx/build"

DOXYGEN_XML_DIR="$PROJECT_ROOT/docs/xml"

cd "$PROJECT_ROOT"

echo "🧹 Cleaning previous build..."

rm -rf "$SPHINX_BUILD"
rm -rf "$PROJECT_ROOT/docs/xml"

echo "📦 Preparing Sphinx assets..."

mkdir -p "$SPHINX_SOURCE/_static"

if [ -d "$PROJECT_ROOT/docs/images" ]; then
    cp -r "$PROJECT_ROOT/docs/images/"* \
          "$SPHINX_SOURCE/_static/" || true
fi

echo "📝 Preparing README for Sphinx..."

cp "$PROJECT_ROOT/README.md" \
   "$SPHINX_SOURCE/README.md"

sed -i \
's|docs/images/|_static/|g' \
"$SPHINX_SOURCE/README.md"

echo "🐍 Activating Python virtual environment..."

source "$PROJECT_ROOT/.venv/bin/activate"

echo "📚 Checking Python dependencies..."

python -m pip install --quiet --upgrade pip

python -m pip install \
    sphinx==7.4.7 \
    pydata-sphinx-theme==0.15.4 \
    breathe==4.36.0 \
    myst-parser==3.0.1 \
    sphinx-design==0.6.1 \
    docutils==0.21.2

echo "📘 Generating Doxygen XML..."

doxygen "$PROJECT_ROOT/docs/doxygen/Doxyfile"

if [ ! -d "$DOXYGEN_XML_DIR" ]; then
    echo "❌ Doxygen XML output not found."
    exit 1
fi

echo "🌐 Building Sphinx documentation..."

sphinx-build \
    -E \
    -a \
    -b html \
    "$SPHINX_SOURCE" \
    "$SPHINX_BUILD"

echo "🚀 Opening documentation..."

xdg-open "$SPHINX_BUILD/index.html" > /dev/null 2>&1 || true

echo "✅ Documentation build completed successfully."
