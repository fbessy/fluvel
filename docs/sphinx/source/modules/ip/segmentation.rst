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

Architecture
------------

The segmentation framework relies on pluggable speed models.

External speed models can be injected into the active contour engine,
while internal regularization is handled by a dedicated internal model.

.. raw:: html

   <div style="text-align:center;">
     <a href="../../../../mermaid/active_contour_speed_models.svg"
        target="_blank">
       <img src="../../../../mermaid/active_contour_speed_models.svg"
            style="max-width:300px; width:100%; height:auto;">
     </a>
   </div>

References
----------

Yonggang Shi and William C. Karl,
*A Real-Time Algorithm for the Approximation of Level-Set-Based Curve Evolution*,
IEEE Transactions on Image Processing, 17(5):645–656, 2008.

Tony F. Chan and Luminita A. Vese,
*Active Contours Without Edges*,
IEEE Transactions on Image Processing, 10(2):266–277, 2001.


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
