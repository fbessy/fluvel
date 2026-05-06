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

# 👉 Choix du compilateur
USE_CLANG=1

# 👉 Exécutable (comme ASAN)
EXECUTABLE_PATH="src/app/Fluvel"

# 👉 Mode (optionnel)
# MODE="FAST"
MODE="DEEP"

#######################################
# CONFIG AUTO
#######################################

if [ "$USE_CLANG" = "1" ]; then
    BUILD_DIR="build-tsan-clang"

    #######################################
    # TSAN OPTIONS (CLANG)
    #######################################

    if [ "$MODE" = "FAST" ]; then
        export TSAN_OPTIONS="\
        halt_on_error=1:\
        report_signal_unsafe=0:\
        ignore_noninstrumented_modules=1"
    else
        export TSAN_OPTIONS="\
        halt_on_error=1:\
        report_signal_unsafe=0:\
        ignore_noninstrumented_modules=1:\
        history_size=7:\
        log_path=tsan_log:\
        suppressions=${SCRIPT_DIR}/tsan.supp"
    fi

else
    BUILD_DIR="build-tsan"

    export TSAN_OPTIONS="halt_on_error=1"
fi

#######################################
# CHECK SUPPRESSION FILE
#######################################

if [ ! -f "${SCRIPT_DIR}/tsan.supp" ]; then
    echo "⚠️ tsan suppression file not found: ${SCRIPT_DIR}/tsan.supp"
fi

#######################################
# RUN
#######################################

EXECUTABLE_FULL_PATH="${PROJECT_ROOT}/${BUILD_DIR}/${EXECUTABLE_PATH}"

echo "======================================"
echo "🚀 Running with TSAN"
echo "Mode      : ${MODE}"
echo "PWD       : $(pwd)"
echo "Build dir : ${BUILD_DIR}"
echo "Exec      : ${EXECUTABLE_FULL_PATH}"
echo "======================================"

if [ ! -d "${PROJECT_ROOT}/${BUILD_DIR}" ]; then
    echo "❌ Build directory not found: ${PROJECT_ROOT}/${BUILD_DIR}"
    echo "👉 Lance d'abord ./tools/build_tsan.sh"
    exit 1
fi

if [ ! -f "${EXECUTABLE_FULL_PATH}" ]; then
    echo "❌ Executable not found: ${EXECUTABLE_FULL_PATH}"
    exit 1
fi

"${EXECUTABLE_FULL_PATH}"
