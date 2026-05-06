#!/usr/bin/env bash

set -e

#######################################
# PATHS ROBUSTES
#######################################

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

#######################################
# CONFIG (MODIFIER ICI UNIQUEMENT)
#######################################

# 👉 Choix du compilateur (Clang par défaut)
USE_CLANG=1
# USE_GCC=1

QT_PATH="$HOME/Qt/6.11.0/gcc_64"
BUILD_TYPE="Debug"

#######################################
# CONFIG AUTO
#######################################

if [ "$USE_CLANG" = "1" ]; then
    BUILD_DIR="build-asan-clang"
    COMPILER_FLAGS="-DCMAKE_CXX_COMPILER=clang++"
    COMPILER_NAME="Clang"
else
    BUILD_DIR="build-asan"
    COMPILER_FLAGS=""
    COMPILER_NAME="GCC"
fi

#######################################
# CLEAN (idempotent)
#######################################

echo "🧹 Cleaning ${PROJECT_ROOT}/${BUILD_DIR}..."
rm -rf "${PROJECT_ROOT}/${BUILD_DIR}"

#######################################
# CONFIGURE
#######################################

echo "⚙️ Configuring..."
echo "Build type : ${BUILD_TYPE}"
echo "Compiler   : ${COMPILER_NAME}"

cmake -S "${PROJECT_ROOT}" -B "${PROJECT_ROOT}/${BUILD_DIR}" \
  -DENABLE_ASAN=ON \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DCMAKE_PREFIX_PATH="${QT_PATH}" \
  ${COMPILER_FLAGS}

#######################################
# BUILD
#######################################

echo "🔨 Building..."
cmake --build "${PROJECT_ROOT}/${BUILD_DIR}" -j$(nproc)

echo "✅ Build terminé (${PROJECT_ROOT}/${BUILD_DIR})"
