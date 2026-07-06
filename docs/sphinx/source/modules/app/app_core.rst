Application Core
================

The Application Core module contains the controllers, runtime services
and shared infrastructure used by the Qt application.

It coordinates image and video processing workflows, manages application
settings, provides diagnostic and utility facilities, and acts as the
bridge between the user interface and the underlying Fluvel IP library.

The module includes image and camera controllers, processing workers,
configuration management, frame-related utilities and editing tools
used throughout the application.

Controllers
-----------

.. doxygenfile:: image_controller.hpp

.. doxygenfile:: image_settings_controller.hpp

.. doxygenfile:: video_controller.hpp


Execution
---------

.. doxygenfile:: active_contour_worker.hpp

.. doxygenfile:: video_active_contour_thread.hpp

.. doxygenfile:: frame_pipeline.hpp

.. doxygenfile:: frame_clock.hpp

.. doxygenfile:: streaming_stats.hpp


Configuration
-------------

.. doxygenfile:: application_settings.hpp

.. doxygenfile:: application_settings_types.hpp


Video
-----

.. doxygenfile:: video_types.hpp

.. doxygenfile:: camera_format_utils.hpp

.. doxygenfile:: video_format_utils.hpp


Image Editing
-------------

.. doxygenfile:: phi_editor.hpp

.. doxygenfile:: shape_type.hpp


Utilities
---------

.. doxygenfile:: image_debug.hpp

.. doxygenfile:: qimage_utils.hpp

.. doxygenfile:: file_utils.hpp

.. doxygenfile:: qcolor_utils.hpp

.. doxygenfile:: device_id_utils.hpp

.. doxygenfile:: time_utils.hpp

