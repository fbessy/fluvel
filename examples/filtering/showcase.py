# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

from pathlib import Path

import cv2
import fluvel


EXAMPLES_DIR = Path(__file__).parent.parent
INPUT_PATH = EXAMPLES_DIR / "resources" / "input.png"
img = cv2.imread(str(INPUT_PATH))

assert img is not None


########################################
# Mean
########################################

out = fluvel.mean(img, radius=1)


########################################
# Median
########################################

# out = fluvel.median(img, radius=2)


########################################
# Morphology
########################################

# out = fluvel.dilate(img, radius=2)

# out = fluvel.erode(img, radius=2)

# out = fluvel.opening(img, radius=2)

# out = fluvel.closing(img, radius=2)

# out = fluvel.top_hat(img, radius=2)

# out = fluvel.black_top_hat(img, radius=2)

# out = fluvel.gradient(img, radius=2)


########################################
# Display
########################################

cv2.imshow("input", img)
cv2.imshow("output", out)

cv2.waitKey(0)
