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

# MODE="FAST"
MODE="DEEP"

# 👉 Choix du compilateur
USE_CLANG=1
# USE_GCC=1

# Exécutable (relatif au build dir)
EXECUTABLE_PATH="src/app/Fluvel"

LSAN_SUPP_FILE="${SCRIPT_DIR}/lsan.supp"

#######################################
# CONFIG AUTO (ne pas modifier)
#######################################

if [ "$USE_CLANG" = "1" ]; then
    BUILD_DIR="build-asan-clang"

    #######################################
    # ASAN (Clang)
    #######################################
    if [ "$MODE" = "FAST" ]; then
        export ASAN_OPTIONS="\
        halt_on_error=1:\
        symbolize=1:\
        detect_leaks=1:\
        fast_unwind_on_malloc=1:\
        log_path=asan_log"
    else
        export ASAN_OPTIONS="\
        halt_on_error=1:\
        symbolize=1:\
        strict_string_checks=1:\
        check_initialization_order=1:\
        detect_stack_use_after_return=1:\
        detect_stack_use_after_scope=1:\
        fast_unwind_on_malloc=0:\
        malloc_context_size=30:\
        detect_odr_violation=1:\
        log_path=asan_log"
    fi

    #######################################
    # LSAN (Clang)
    #######################################
    export LSAN_OPTIONS="\
    detect_leaks=1:\
    report_objects=1:\
    suppressions=${LSAN_SUPP_FILE}"

    # Vérif optionnelle
    if [ ! -f "${LSAN_SUPP_FILE}" ]; then
        echo "⚠️ LSAN suppression file not found: ${LSAN_SUPP_FILE}"
    fi

else
    BUILD_DIR="build-asan"

    #######################################
    # ASAN (GCC)
    #######################################
    export ASAN_OPTIONS="\
    halt_on_error=1:\
    detect_leaks=1:\
    symbolize=1:\
    strict_string_checks=1:\
    check_initialization_order=1:\
    detect_stack_use_after_return=1:\
    log_path=asan_log"
fi

#######################################
# SYMBOLIZER (important pour Clang)
#######################################

if command -v llvm-symbolizer >/dev/null 2>&1; then
    export ASAN_SYMBOLIZER_PATH="$(command -v llvm-symbolizer)"
fi

#######################################
# RUN
#######################################

EXECUTABLE_FULL_PATH="${PROJECT_ROOT}/${BUILD_DIR}/${EXECUTABLE_PATH}"

echo "======================================"
echo "🚀 Running with ASAN"
echo "Mode      : ${MODE}"
echo "PWD       : $(pwd)"
echo "Build dir : ${BUILD_DIR}"
echo "Exec      : ${EXECUTABLE_FULL_PATH}"
echo "======================================"

if [ ! -d "${PROJECT_ROOT}/${BUILD_DIR}" ]; then
    echo "❌ Build directory not found: ${PROJECT_ROOT}/${BUILD_DIR}"
    echo "👉 Lance d'abord ./tools/build_asan.sh"
    exit 1
fi

if [ ! -f "${EXECUTABLE_FULL_PATH}" ]; then
    echo "❌ Executable not found: ${EXECUTABLE_FULL_PATH}"
    exit 1
fi

"${EXECUTABLE_FULL_PATH}"
