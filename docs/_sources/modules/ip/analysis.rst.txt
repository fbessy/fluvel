Shape Analysis
==============

This module provides geometric representations and algorithms used for
shape comparison and contour analysis.

It includes utilities for converting contour representations, computing
geometric properties and evaluating similarity measures such as the
Hausdorff distance.

These components are primarily intended for contour validation,
benchmarking and shape-based analysis workflows.

References
----------

Abdel Aziz Taha and Allan Hanbury,
*An Efficient Algorithm for Calculating the Exact Hausdorff Distance*,
IEEE Transactions on Pattern Analysis and Machine Intelligence,
37(11):2153–2163, 2015.

.. doxygenfile:: shape.hpp

.. doxygenfile:: shape_conversion.hpp

.. doxygenfile:: hausdorff_distance.hpp
