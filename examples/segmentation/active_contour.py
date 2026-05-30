# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).parent.parent))

import utils

import cv2
import fluvel
import time


img = utils.load_image("input.png")

phi = utils.load_image("initial_phi.png")

phi2 = utils.load_image("initial_phi2.png")

phi = utils.resize_phi_to_image(img, phi)
phi2 = utils.resize_phi_to_image(img, phi2)

# Run segmentation
start = time.time()

lists0 = fluvel.active_contour(img)
lists1 = fluvel.active_contour(img, phi)
lists2 = fluvel.active_contour(img, phi2)

elapsed = time.time() - start

print(f"Segmentation time: {elapsed:.3f}s")


display_result0 = utils.draw_contours(img, lists0)
display_result1 = utils.draw_contours(img, lists1)
display_result2 = utils.draw_contours(img, lists2)


cv2.imshow("Segmentation0", display_result0)
cv2.imshow("Segmentation1", display_result1)
cv2.imshow("Segmentation2", display_result2)

cv2.waitKey(0)
