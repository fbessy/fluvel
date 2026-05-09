#!/usr/bin/env bash

set -e

COMMIT_SHORT=${GITHUB_SHA::8}
BRANCH_NAME=${GITHUB_REF_NAME}
BUILD_DATE=$(date -u +"%Y-%m-%dT%H:%M:%SZ")

cat > latest.json <<EOF
{
  "version": "$VERSION",
  "branch": "$BRANCH_NAME",
  "commit": "$COMMIT_SHORT",
  "date": "$BUILD_DATE",

  "windows": {
    "installer": "https://github.com/fbessy/fluvel/releases/latest/download/Fluvel-win64-installer.exe",
    "portable": "https://github.com/fbessy/fluvel/releases/latest/download/Fluvel-win64-portable.zip"
  },

  "macos": {
    "dmg": "https://github.com/fbessy/fluvel/releases/latest/download/Fluvel-macos.dmg"
  },

  "linux": {
    "appimage": "https://github.com/fbessy/fluvel/releases/latest/download/Fluvel-linux.AppImage",
    "flatpak": "https://github.com/fbessy/fluvel/releases/latest/download/Fluvel-linux.flatpak"
  },

  "checksums": $ALL_CHECKSUMS
}
EOF
