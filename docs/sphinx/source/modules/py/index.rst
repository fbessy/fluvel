Python Bindings
===============

The Python bindings provide access to Fluvel IP from Python through a
thin pybind11-based interoperability layer.

They expose the main image processing, segmentation and analysis
capabilities of the library while preserving the performance and API
semantics of the underlying C++ implementation.

The bindings are organized into two complementary areas:

- **Python Interoperability**: type conversions and integration utilities
  used to bridge Python and C++.
- **Python Bindings**: public Python API exposing Fluvel IP features to
  Python applications and scripts.

.. toctree::
   :maxdepth: 1

   python_interop
   python_bindings
