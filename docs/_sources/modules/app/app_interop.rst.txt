Application Interoperability
============================

This module provides the adaptation layer between the Qt application
and the underlying Fluvel IP image processing library.

It contains conversion utilities responsible for translating application
types and third-party data structures into the internal representations
used by Fluvel IP, and vice versa.

These adapters help maintain a clear separation between the user interface,
external dependencies and the core processing components.

.. doxygenfile:: image_adapters.hpp

.. doxygenfile:: color_adapters.hpp

.. doxygenfile:: contour_adapters.hpp
