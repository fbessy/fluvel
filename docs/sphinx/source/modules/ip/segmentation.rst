Segmentation
============

This module contains the active contour framework used by Fluvel IP for
image segmentation.

It provides contour representations, contour evolution algorithms,
speed models and initialization utilities used to drive level-set based
segmentation workflows.

The framework is designed around a separation between contour evolution,
speed computation and initialization, making it possible to combine
different segmentation strategies while reusing the same underlying
infrastructure.


Active Contours
---------------

.. doxygenfile:: contour_data.hpp

.. doxygenfile:: contour_types.hpp

.. doxygenfile:: active_contour.hpp

.. doxygenfile:: active_contour_types.hpp

.. doxygenfile:: majority_internal_speed.hpp

.. doxygenfile:: contour_diagnostics.hpp


Speed Models
------------

.. doxygenfile:: speed_model_types.hpp

.. doxygenfile:: speed_model.hpp

.. doxygenfile:: region_gray_speed_model.hpp

.. doxygenfile:: region_color_speed_model.hpp


Contour Initialization
----------------------

.. doxygenfile:: boundary_factory.hpp
