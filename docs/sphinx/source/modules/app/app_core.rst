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

.. doxygenfile:: image_processing_worker.hpp

.. doxygenfile:: video_processing_thread.hpp

.. doxygenfile:: frame_pipeline.hpp

.. doxygenfile:: frame_clock.hpp

.. doxygenfile:: streaming_stats.hpp


Configuration
-------------

.. doxygenfile:: application_settings.hpp

.. doxygenfile:: application_settings_types.hpp


Video Processing
----------------

.. doxygenfile:: video_types.hpp

.. doxygenfile:: camera_format_utils.hpp

.. doxygenfile:: video_format_utils.hpp

.. doxygenfile:: frame_rendering_utils.hpp


Recording
---------

.. doxygenfile:: recording_types.hpp

.. doxygenfile:: recording_buffer_settings.hpp

.. doxygenfile:: video_frame_header.hpp

.. doxygenfile:: video_frame_buffer.hpp

.. doxygenfile:: video_frame_spool.hpp

.. doxygenfile:: video_recorder_worker.hpp


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

