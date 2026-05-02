<p align="center">
  <img src="docs/images/fluvel.svg" width="200">
</p>

<h1 align="center">Fluvel</h1>

<p align="center">
  Real-time image segmentation and active contour experimentation platform
</p>

---

Fluvel is a research-oriented image segmentation application based on  
region-based active contour (level-set) methods.

It provides a modern C++ engine for fast experimentation, real-time visualization,  
and reproducible algorithm evaluation.

The project is currently under active development.

---

## 🚀 Get Fluvel

👉 **Latest builds (all platforms):**  
https://fbessy.github.io/fluvel/

---

## 📚 Documentation

👉 https://fabienip.gitlab.io/fluvel/

---

## ✨ Features

- Region-based active contour evolution  
- Real-time image processing  
- Video stream support  
- Modular architecture for feature extensions  
- Qt-based visualization interface  
- Reproducible builds (CMake, Flatpak, AppImage)  

---

## 📸 Screenshot

![Fluvel UI](web/images/screenshot.png)

---

## 🧠 Overview

Fluvel implements region-based active contour algorithms operating on  
row-major image buffers.

The project focuses on:

- A clear and maintainable implementation of active contour models  
- A modern C++ architecture  
- Separation between processing engine and visualization layer  
- Reusability as a standalone library  

The processing core does not depend on Qt.  
Qt is used only for visualization and interaction.

---

## 🏗️ Architecture

The project is organized into:

- `src/` — Core engine and application code  
- `docs/` — Documentation configuration (Doxygen)  
- `CMakeLists.txt` — Build configuration  
- `packaging/` — Packaging (Flatpak, AppImage, etc.)  

The image processing engine operates on contiguous memory buffers.  
Visualization and UI components are separated from the core algorithmic logic.

---

## 📦 Modules

- **fluvel_app** — Application layer (UI and orchestration)  
- **fluvel_ip** — Image processing engine (algorithms and data processing)  

---

## 🛠️ Build

### Requirements

- CMake ≥ 3.x  
- Clang (recommended)  
- Qt6 (for UI build)  

---

### Build with CMake

```bash
cmake -S . -B build
cmake --build build
./build/Fluvel
