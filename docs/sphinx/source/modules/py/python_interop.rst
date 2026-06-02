Python Interoperability
=======================

This module provides the conversion layer between Python objects and
the native data structures used by Fluvel IP.

It contains utilities responsible for translating NumPy arrays,
contours and other Python-facing representations into their C++
counterparts, and vice versa.

These components form the foundation of the Python bindings while
keeping conversion logic isolated from the public API.

.. doxygenfile:: py_array_conversion.hpp

.. doxygenfile:: contour_conversion.hpp
