UI
==

The user interface layer provides the visual components of the Qt
application.

It includes application windows, dialogs and reusable widgets used to
build image and video processing workflows while remaining separated
from the underlying processing and controller logic.

Image Viewer Architecture
-------------------------

The ImageViewerWidget delegates user interactions to an
ImageViewerInteraction implementation. Interactions are composed
from independent behaviors such as panning, color picking,
drag-and-drop and fullscreen support.

.. raw:: html

   <div style="text-align:center;">
     <a href="/fluvel/mermaid/image_viewer_behaviors.svg"
        target="_blank">
       <img src="/fluvel/mermaid/image_viewer_behaviors.svg"
            style="max-width:300px; width:100%; height:auto;">
     </a>
   </div>

.. toctree::
   :maxdepth: 1

   windows
   components
