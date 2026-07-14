Media
=====

The Media module provides video export infrastructure and media backend
integrations used by the Fluvel application.

It defines the generic video export API, export settings and backend
abstractions, and provides FFmpeg-based video encoding support when
available.


Video Export
------------

.. doxygenfile:: video_export_settings.hpp

.. doxygenfile:: video_exporter_backend.hpp

.. doxygenfile:: video_exporter.hpp

.. doxygenfile:: video_exporter_utils.hpp


FFmpeg Backend
--------------

.. doxygenfile:: ffmpeg_video_exporter.hpp

.. doxygenfile:: ffmpeg_codec_utils.hpp

.. doxygenfile:: ffmpeg_utils.hpp
