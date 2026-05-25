# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

from pathlib import Path

import cv2
import fluvel

import time

EXAMPLES_DIR = Path(__file__).parent.parent
INPUT_PATH = EXAMPLES_DIR / "resources" / "input.png"
img = cv2.imread(str(INPUT_PATH), cv2.IMREAD_UNCHANGED)

if img is None:
    raise RuntimeError(
        f"Cannot load image: {INPUT_PATH}"
    )

# Run segmentation
start = time.time()

result = fluvel.active_contour(img)

elapsed = time.time() - start

print(f"Segmentation time: {elapsed:.3f}s")


# Draw contours
display = img.copy()

bgr_out = (255, 0, 64)
bgr_in = (118, 230, 0)

display[
    result.outer[:, 1],
    result.outer[:, 0]
] = bgr_out

display[
    result.inner[:, 1],
    result.inner[:, 0]
] = bgr_in


cv2.imshow("Segmentation", display)

cv2.waitKey(0)
