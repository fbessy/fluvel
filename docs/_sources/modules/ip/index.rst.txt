Image Processing Library
========================

Fluvel IP is the core image processing library of the Fluvel project.

It provides filtering, segmentation and shape-analysis algorithms designed
for both interactive applications and scripting workflows. The library is
implemented in modern C++ and remains independent from Qt whenever possible.

The codebase is organized into four main areas:

- **Filtering**: image preprocessing, denoising and enhancement algorithms.
- **Segmentation**: active contour models and level-set based segmentation tools.
- **Analysis**: shape analysis, contour metrics and image measurements.
- **Core**: shared data structures, image abstractions and common infrastructure.

.. toctree::
   :maxdepth: 1

   filtering
   segmentation
   analysis
   ip_core
