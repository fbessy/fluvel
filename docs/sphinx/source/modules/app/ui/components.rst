UI Components
=============

This module contains the reusable user interface components used throughout
the Fluvel application.

It includes configuration widgets, image viewing components, overlays,
interaction behaviors and supporting UI infrastructure used to build image
and video processing workflows.

The image viewer is designed around a composable interaction model, where
navigation, visualization and editing features are implemented as
independent behaviors that can be combined and extended.

Configuration
-------------

.. doxygenfile:: algo_settings_widget.hpp

.. doxygenfile:: display_settings_widget.hpp

.. doxygenfile:: analysis_widget.hpp

.. doxygenfile:: color_selector_widget.hpp

.. doxygenfile:: kernel_size_spinbox.hpp

.. doxygenfile:: configuration_actions_widget.hpp


Image Viewer
------------

.. doxygenfile:: image_viewer_widget.hpp

.. doxygenfile:: image_viewer_listener.hpp

.. doxygenfile:: image_viewer_interaction.hpp

.. doxygenfile:: interaction_set.hpp

.. doxygenfile:: image_viewer_behavior.hpp

.. doxygenfile:: initialization_behavior.hpp

.. doxygenfile:: fullscreen_behavior.hpp

.. doxygenfile:: autofit_behavior.hpp

.. doxygenfile:: pan_behavior.hpp

.. doxygenfile:: color_picker_behavior.hpp

.. doxygenfile:: pixel_info_behavior.hpp

.. doxygenfile:: drag_drop_behavior.hpp


Overlays
--------

..
   Temporarily excluded because current Breathe releases do not support
   Qt Q_PROPERTY members.

   .. doxygenfile:: overlay_text_item.hpp

.. doxygenfile:: hud_overlay_controller.hpp

.. doxygenfile:: pixel_info_overlay.hpp

.. doxygenfile:: mini_map_widget.hpp


Video Playback
--------------

.. doxygenfile:: timeline_slider.hpp

.. doxygenfile:: jump_slider.hpp

.. doxygenfile:: styled_slider.hpp

.. doxygenfile:: volume_slider.hpp

.. doxygenfile:: volume_controller.hpp

.. doxygenfile:: video_shortcut_manager.hpp


Capture
-------

.. doxygenfile:: capture_controls_widget.hpp

.. doxygenfile:: capture_stats_utils.hpp


Fullscreen Controls
-------------------

.. doxygenfile:: fullscreen_image_control_bar.hpp

.. doxygenfile:: fullscreen_video_control_bar.hpp


Animated Controls
-----------------

.. doxygenfile:: styled_tool_button.hpp

.. doxygenfile:: animated_push_button.hpp

.. doxygenfile:: animated_tab_widget.hpp

.. NOTE::
   The following files are temporarily excluded because current
   Breathe releases do not support Qt Q_PROPERTY members.

..
   .. doxygenfile:: animated_icon.hpp
   .. doxygenfile:: scale_animation.hpp
   .. doxygenfile:: checked_animation.hpp
   .. doxygenfile:: animated_types.hpp


Custom Graphics Items
---------------------

.. doxygenfile:: contour_point_item.hpp

.. doxygenfile:: phi_view_model.hpp


Miscellaneous Widgets
---------------------

.. doxygenfile:: right_panel_toggle_button.hpp

.. doxygenfile:: clickable_label.hpp


UI Infrastructure
-----------------

.. doxygenfile:: icon_loader.hpp

.. doxygenfile:: qt_utils.hpp

.. doxygenfile:: ui_appearance.hpp

..
   Temporarily excluded due to Doxygen/Breathe limitations when
   documenting global UI theme constants.

   .. doxygenfile:: ui_theme.hpp

.. doxygenfile:: slider_style.hpp


UI Theme
========

The UI theme centralizes the colors, metrics and animation constants
used throughout the application.

It defines:

- Accent colors
- Text colors
- Panel colors
- Control colors
- Slider colors
- Layout metrics
- Animation durations

