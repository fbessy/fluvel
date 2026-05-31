# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).parent.parent))

import utils

import csv
import shutil
import time

import cv2
import fluvel


dataset_dir = Path(__file__).parent.parent / "resources" / "dataset_demo"

output_dir = Path(__file__).parent / "output" / "dataset_demo"

overlays_dir = output_dir / "overlays"
masks_dir = output_dir / "masks"

shutil.rmtree(output_dir, ignore_errors=True)

overlays_dir.mkdir(parents=True)
masks_dir.mkdir(parents=True)

rows = []

for image_path in sorted(dataset_dir.glob("*.png")):
    if image_path.stem.endswith("_gt"):
        continue

    gt_path = image_path.with_name(image_path.stem + "_gt.png")

    if not gt_path.exists():
        print(f"Skipping {image_path.name}: ground truth not found")
        continue

    print(f"Processing {image_path.name}")

    #
    # Input image
    #

    image = utils.load_image(str(image_path))

    #
    # Optional:
    # Uncomment to evaluate robustness to Gaussian noise.
    #
    # image = fluvel.gaussian_noise(image, sigma=10)
    #

    #
    # Ground truth
    #

    gt_image = utils.load_image(str(gt_path))

    mask_gt = utils.weizmann_gt_to_mask(gt_image)

    cv2.imwrite(str(masks_dir / f"{image_path.stem}_mask.png"), mask_gt)

    gt_contours = fluvel.find_contours(mask_gt)

    #
    # Segmentation
    #

    start = time.time()

    result = fluvel.active_contour(image)

    elapsed_ms = (time.time() - start) * 1000.0

    #
    # Metrics
    #

    metrics = fluvel.hausdorff_metrics(gt_contours.outer, result.outer, quantile=95)

    #
    # Overlay
    #

    overlay = image.copy()

    overlay = utils.draw_contours(
        overlay, gt_contours, bgr_out=(0, 255, 0), bgr_in=(0, 180, 0)
    )

    overlay = utils.draw_contours(
        overlay, result, bgr_out=(0, 0, 255), bgr_in=(0, 255, 255)
    )

    cv2.imwrite(str(overlays_dir / f"{image_path.stem}_overlay.png"), overlay)

    rows.append(
        [
            image_path.stem,
            metrics.distance,
            metrics.modified_distance,
            metrics.quantile_distance,
            metrics.centroids_distance,
            elapsed_ms,
        ]
    )

#
# CSV report
#

csv_path = output_dir / "metrics.csv"

with open(csv_path, "w", newline="") as f:
    writer = csv.writer(f)

    writer.writerow(
        [
            "image",
            "hausdorff",
            "modified_hausdorff",
            "hd95",
            "centroid_distance",
            "runtime_ms",
        ]
    )

    writer.writerows(rows)

print()
print(f"Results written to: {output_dir}")
print(f"CSV report: {csv_path}")
