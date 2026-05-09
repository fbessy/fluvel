#!/usr/bin/env bash

set -e

EXPECTED_COMMIT="${GITHUB_SHA}"

echo "Expected workflow commit:"
echo "$EXPECTED_COMMIT"

echo ""
echo "Windows:"
cat windows/commit-windows.txt

echo ""
echo "macOS:"
cat macos/commit-macos.txt

echo ""
echo "Linux AppImage:"
cat linux/commit-linux-appimage.txt

echo ""
echo "Linux Flatpak:"
cat linux/commit-linux-flatpak.txt

UNIQUE_COMMITS=$(
  for file in \
    windows/commit-windows.txt \
    macos/commit-macos.txt \
    linux/commit-linux-appimage.txt \
    linux/commit-linux-flatpak.txt
  do
    tr -d '\r\n' < "$file"
    echo
  done | sort -u | wc -l
)

if [ "$UNIQUE_COMMITS" -ne 1 ]; then
  echo "ERROR: Commit mismatch between release artifacts"
  exit 1
fi

for file in \
  windows/commit-windows.txt \
  macos/commit-macos.txt \
  linux/commit-linux-appimage.txt \
  linux/commit-linux-flatpak.txt
do
  CURRENT_COMMIT=$(tr -d '\r\n' < "$file")

  if [ "$CURRENT_COMMIT" != "$EXPECTED_COMMIT" ]; then
    echo "ERROR: Release artifacts were not built from the current workflow commit"
    echo "Expected: $EXPECTED_COMMIT"
    echo "Found   : $CURRENT_COMMIT"
    exit 1
  fi
done

echo ""
echo "Commit consistency verified ✔"
