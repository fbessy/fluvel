FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# =========================
# SYSTEM DEPENDENCIES
# =========================
RUN apt update && apt install -y \
    build-essential \
    cmake \
    ninja-build \
    clang \
    git \
    curl \
    wget \
    file \
    rsync \
    python3 \
    python3-pip \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libglib2.0-0 \
    libxkbcommon0 \
    libxkbcommon-x11-0 \
    libxcb1 \
    libxcb-util1 \
    libxcb-cursor0 \
    libxcb-icccm4 \
    libxcb-keysyms1 \
    libxcb-render-util0 \
    libxcb-image0 \
    libxcb-xinerama0 \
    libxcb-shape0 \
    libxcb-randr0 \
    libxrandr2 \
    libpulse0 \
    libdbus-1-3 \
    libx11-6 \
    libxext6 \
    libxrender1 \
    libxi6 \
    libxfixes3 \
    libxcomposite1 \
    libxcursor1 \
    libxdamage1 \
    libxtst6 \
    libsm6 \
    libice6 \
    libfontconfig1 \
    libfreetype6 \
    libegl1 \
    libwayland-client0 \
    && apt clean

# =========================
# PYTHON + AQT
# =========================
RUN pip install aqtinstall --break-system-packages

# =========================
# INSTALL QT (CACHÉ DANS L’IMAGE)
# =========================
ENV QT_VERSION=6.9.1
ENV QT_DIR=/opt/Qt

RUN python3 -m aqt install-qt linux desktop ${QT_VERSION} linux_gcc_64 \
    -O ${QT_DIR} \
    -m qtmultimedia

ENV QT_ROOT=${QT_DIR}/${QT_VERSION}/gcc_64
ENV PATH="${QT_ROOT}/bin:${PATH}"
ENV CMAKE_PREFIX_PATH="${QT_ROOT}/lib/cmake"
ENV Qt6_DIR="${QT_ROOT}/lib/cmake/Qt6"

# =========================
# LINUXDEPLOY
# =========================
RUN mkdir -p /opt/tools && \
    wget -O /opt/tools/linuxdeploy https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage && \
    wget -O /opt/tools/linuxdeploy-plugin-qt https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage && \
    chmod +x /opt/tools/*

ENV LINUXDEPLOY_PLUGIN_PATH=/opt/tools

WORKDIR /workspace
