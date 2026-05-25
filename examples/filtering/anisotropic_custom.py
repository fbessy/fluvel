# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

from pathlib import Path

import cv2
import fluvel

EXAMPLES_DIR = Path(__file__).parent.parent
INPUT_PATH = EXAMPLES_DIR / "resources" / "input.png"
img = cv2.imread(str(INPUT_PATH))

assert img is not None

params = fluvel.AnisoParams()
params.iterations = 30
params.kappa = 10
params.lambda_ = 0.12

out = fluvel.anisotropic_diffusion(img, params)

cv2.imshow("input", img)
cv2.imshow("output", out)

cv2.waitKey(0)
