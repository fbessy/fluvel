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

fast = fluvel.AnisoParams()
fast.iterations = 5

strong = fluvel.AnisoParams()
strong.iterations = 40

out1 = fluvel.anisotropic_diffusion(img, fast)
out2 = fluvel.anisotropic_diffusion(img, strong)

cv2.imshow("input", img)

cv2.imshow("fast", out1)
cv2.imshow("strong", out2)

cv2.waitKey(0)
