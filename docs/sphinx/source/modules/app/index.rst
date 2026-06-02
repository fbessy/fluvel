Qt Application
==============

The Qt application provides the graphical user interface and runtime
infrastructure of Fluvel.

It is responsible for user interaction, session management, camera and
image workflows, and integration with the underlying Fluvel IP image
processing library.

The application is organized into three main areas:

- **UI**: windows, dialogs and reusable user interface components.
- **Application Core**: controllers, settings management and execution workflows.
- **Interoperability**: adapters and integration layers connecting the application
  to external libraries and processing backends.

.. toctree::
   :maxdepth: 2

   ui/index
   app_core
   app_interop
   ui
