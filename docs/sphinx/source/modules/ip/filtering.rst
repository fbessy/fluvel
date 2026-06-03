Image Filtering
===============

This module provides image preprocessing and enhancement algorithms used
throughout Fluvel IP.

The filtering framework includes spatial and temporal smoothing filters,
edge-preserving diffusion methods, morphological operators and reusable
processing pipelines. These algorithms are commonly used to prepare images
for segmentation and shape-analysis tasks.


References
----------

Pietro Perona and Jitendra Malik,
*Scale-Space and Edge Detection Using Anisotropic Diffusion*,
IEEE Transactions on Pattern Analysis and Machine Intelligence,
12(7):629–639, 1990.

Simon Perreault and Patrick Hébert,
*Median Filtering in Constant Time*,
IEEE Transactions on Image Processing,
16(9):2389–2394, 2007.

Marcel van Herk,
*A Fast Algorithm for Local Minimum and Maximum Filters on Rectangular and Octagonal Kernels*,
Pattern Recognition Letters,
13(7):517–521, 1992.


Related Components
------------------

- :cpp:class:`fluvel_ip::filter::TemporalMean`


Filter
------

.. doxygennamespace:: fluvel_ip::filter
   :members:

Morphology
----------

.. doxygennamespace:: fluvel_ip::filter::morpho
   :members:

Noise
-----

.. doxygennamespace:: fluvel_ip::noise
   :members:

Pixel-wise Operations
---------------------

.. doxygennamespace:: fluvel_ip::pixelwise
   :members:

Pipeline
--------

.. doxygenclass:: fluvel_ip::ImagePipeline
   :members:

.. doxygenstruct:: fluvel_ip::ProcessingParams
   :members:
