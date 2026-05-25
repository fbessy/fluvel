# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

from pathlib import Path

import cv2
import fluvel


EXAMPLES_DIR = Path(__file__).parent.parent

INPUT_PATH = EXAMPLES_DIR / "resources" / "input.png"


img = cv2.imread(str(INPUT_PATH))

if img is None:
    raise RuntimeError(
        f"Cannot load image: {INPUT_PATH}"
    )


########################################
# Select filter
########################################

out = fluvel.gaussian_noise(img)

# out = fluvel.gaussian_noise(img, sigma=40)

# out = fluvel.impulsive_noise(img)

# out = fluvel.impulsive_noise(img, probability=0.10)

# out = fluvel.speckle_uniform(img)

# out = fluvel.speckle_gamma(img, sigma=0.30)


########################################
# Display
########################################

cv2.imshow("input", img)

cv2.imshow("output", out)

cv2.waitKey(0)
