Image Filtering
===============

This module provides image preprocessing and enhancement algorithms used
throughout Fluvel IP.

The filtering framework includes spatial and temporal smoothing filters,
edge-preserving diffusion methods, morphological operators and reusable
processing pipelines. These algorithms are commonly used to prepare images
for segmentation and shape-analysis tasks.


Smoothing
---------

.. doxygenfile:: mean_filter.hpp
.. doxygenfile:: mean_filter_3x3.hpp
.. doxygenfile:: mean_filter_sliding.hpp
.. doxygenfile:: median_filter.hpp
.. doxygenfile:: temporal_mean.hpp


Diffusion
---------

.. doxygenfile:: anisotropic_diffusion.hpp


Morphology
----------

.. doxygenfile:: morpho.hpp
.. doxygenfile:: naive_morpho.hpp
.. doxygenfile:: van_herk_morpho.hpp


Pixel Operations
----------------

.. doxygenfile:: noise.hpp
.. doxygenfile:: pixel_wise.hpp


Pipeline
--------

.. doxygenfile:: image_pipeline.hpp
.. doxygenfile:: processing_params.hpp
