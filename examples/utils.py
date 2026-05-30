# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

from pathlib import Path

import cv2

import numpy as np


def load_image(filename, flags=cv2.IMREAD_UNCHANGED):
    """
    Load an image from the local resources directory.

    The image is loaded using OpenCV from the ``resources`` directory
    located next to the current Python module.

    Parameters
    ----------
    filename : str or pathlib.Path
        Name or relative path of the image file inside the
        ``resources`` directory.

    flags : int, optional
        OpenCV image loading flags passed to ``cv2.imread``.
        Default is ``cv2.IMREAD_UNCHANGED``.

        Common values include:

        - ``cv2.IMREAD_COLOR``
        - ``cv2.IMREAD_GRAYSCALE``
        - ``cv2.IMREAD_UNCHANGED``

    Returns
    -------
    numpy.ndarray
        Loaded image as a NumPy array.

    Raises
    ------
    RuntimeError
        Raised if the image cannot be loaded.

    Examples
    --------
    Load a color image:

    >>> image = load_image("lena.png")

    Load a grayscale image:

    >>> image = load_image(
    ...     "lena.png",
    ...     cv2.IMREAD_GRAYSCALE
    ... )
    """
    resources = Path(__file__).parent / "resources"

    path = resources / filename

    image = cv2.imread(str(path), flags)

    if image is None:
        raise RuntimeError(f"Cannot load image: {path}")

    return image


def resize_phi_to_image(img, phi):
    """
    Resize a level-set image to match an image shape without interpolation.

    This function ensures that ``phi`` has the same spatial dimensions
    as ``img`` while preserving the exact discrete structure of the
    level-set representation.

    No interpolation, filtering, antialiasing, or geometric resampling
    is performed. The content is copied directly into a new buffer with
    matching dimensions.

    Behavior
    --------
    - If ``phi`` already has the same dimensions as ``img``,
      it is returned unchanged.
    - If ``phi`` is smaller, the overlapping region is copied and the
      remaining area is initialized to zero.
    - If ``phi`` is larger, it is cropped to match the image size.

    Parameters
    ----------
    img : numpy.ndarray
        Reference image whose dimensions must be matched.

    phi : numpy.ndarray
        Discrete level-set image.

    Returns
    -------
    numpy.ndarray
        A level-set image with the same dimensions as ``img``.

    Notes
    -----
    This function is intended for discrete active contour and level-set
    processing where interpolation would corrupt the contour topology
    or neighborhood connectivity.
    """

    img_h, img_w = img.shape[:2]
    phi_h, phi_w = phi.shape[:2]

    if (img_h, img_w) == (phi_h, phi_w):
        return phi

    resized = np.zeros((img_h, img_w), dtype=phi.dtype)

    copy_h = min(img_h, phi_h)
    copy_w = min(img_w, phi_w)

    resized[:copy_h, :copy_w] = phi[:copy_h, :copy_w]

    return resized


def draw_contours(image, result, bgr_out=(255, 0, 64), bgr_in=(118, 230, 0)):
    """
    Draw active contour boundaries on an image.

    Parameters
    ----------
    image
        Input image (Gray8 or BGR).

    result
        ActiveContourResult.

    bgr_out
        Outer contour color.

    bgr_in
        Inner contour color.

    Returns
    -------
    numpy.ndarray
        Colored output image.
    """

    if image.ndim == 2:
        display = cv2.cvtColor(image, cv2.COLOR_GRAY2BGR)
    else:
        display = image.copy()

    display[result.outer[:, 1], result.outer[:, 0]] = bgr_out

    display[result.inner[:, 1], result.inner[:, 0]] = bgr_in

    return display
