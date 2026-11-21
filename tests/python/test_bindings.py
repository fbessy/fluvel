# SPDX-License-Identifier: CeCILL-2.1
# Copyright (C) 2010-2026 Fabien Bessy

"""
Tests of the Fluvel Python bindings.

Run from the repository root, with the directory containing the compiled
module in the Python path (for example after scripts/build_bindings.sh):

    PYTHONPATH=build-bindings/python python -m pytest tests/python
"""

import threading
import time

import numpy as np
import pytest

fluvel = pytest.importorskip("fluvel")


def make_gray(height=96, width=128, seed=0):
    """Dark background with a bright disk and some noise."""
    rng = np.random.default_rng(seed)

    y, x = np.ogrid[:height, :width]
    disk = (x - width // 2) ** 2 + (y - height // 2) ** 2 < 30**2

    image = np.where(disk, 200, 40).astype(np.int16)
    image += rng.integers(-10, 11, size=image.shape)

    return np.clip(image, 0, 255).astype(np.uint8)


def make_bgr(**kwargs):
    gray = make_gray(**kwargs)
    return np.dstack([gray, gray // 2, gray // 3])


#
# Input arrays
#


@pytest.mark.parametrize(
    "make_view",
    [
        lambda img: img[..., ::-1],  # BGR <-> RGB
        lambda img: img[:, ::2],
        lambda img: img[::-1],
        lambda img: img[::2],
        lambda img: img[..., 0],  # one channel of a color image
        lambda img: np.asfortranarray(img),
        lambda img: img.transpose(1, 0, 2),
    ],
)
def test_non_contiguous_color_input_gives_same_result_as_a_copy(make_view):
    view = make_view(make_bgr())
    expected = fluvel.mean(np.ascontiguousarray(view), radius=1)

    assert np.array_equal(fluvel.mean(view, radius=1), expected)


def test_non_contiguous_gray_input_gives_same_result_as_a_copy():
    gray = make_gray()

    for view in (gray.T, gray[:, ::2], gray[::-1]):
        expected = fluvel.median(np.ascontiguousarray(view), radius=1)

        assert np.array_equal(fluvel.median(view, radius=1), expected)


@pytest.mark.parametrize(
    "dtype", [np.float32, np.float64, np.uint16, np.int32, np.int8, np.bool_]
)
def test_only_uint8_images_are_accepted(dtype):
    image = make_gray().astype(dtype)

    with pytest.raises(TypeError, match="uint8"):
        fluvel.mean(image, radius=1)

    with pytest.raises(TypeError, match="uint8"):
        fluvel.active_contour(image)

    with pytest.raises(TypeError, match="uint8"):
        fluvel.find_contours(image)


@pytest.mark.parametrize(
    "shape", [(10,), (4, 4, 3, 1), (8, 8, 2), (8, 8, 5), (0, 8), (8, 0)]
)
def test_invalid_shapes_are_rejected(shape):
    with pytest.raises(ValueError):
        fluvel.mean(np.zeros(shape, dtype=np.uint8), radius=1)


#
# Output shape
#


@pytest.mark.parametrize("channels", [None, 1, 3, 4])
def test_output_has_the_shape_of_the_input(channels):
    shape = (40, 50) if channels is None else (40, 50, channels)
    image = np.random.default_rng(1).integers(0, 256, shape, dtype=np.uint8)

    functions = [
        lambda a: fluvel.mean(a, 1),
        lambda a: fluvel.median(a, 1),
        lambda a: fluvel.dilate(a, 1),
        lambda a: fluvel.erode(a, 1),
        lambda a: fluvel.opening(a, 1),
        lambda a: fluvel.closing(a, 1),
        lambda a: fluvel.top_hat(a, 1),
        lambda a: fluvel.black_top_hat(a, 1),
        lambda a: fluvel.gradient(a, 1),
        lambda a: fluvel.anisotropic_diffusion(a),
        lambda a: fluvel.gaussian_noise(a),
        lambda a: fluvel.impulsive_noise(a),
        lambda a: fluvel.speckle_uniform(a),
        lambda a: fluvel.speckle_gamma(a),
    ]

    for function in functions:
        out = function(image)

        assert out.shape == image.shape
        assert out.dtype == np.uint8


def test_filters_can_be_chained_on_gray_images():
    gray = make_gray()

    out = fluvel.median(fluvel.mean(gray, radius=1), radius=1)

    assert out.shape == gray.shape


#
# Noise
#


@pytest.mark.parametrize(
    "noise",
    [
        lambda a, s: fluvel.gaussian_noise(a, sigma=20, seed=s),
        lambda a, s: fluvel.impulsive_noise(a, probability=0.1, seed=s),
        lambda a, s: fluvel.speckle_uniform(a, sigma=0.2, seed=s),
        lambda a, s: fluvel.speckle_gamma(a, sigma=0.2, seed=s),
    ],
)
def test_noise_is_reproducible_with_a_seed(noise):
    image = make_gray()

    assert np.array_equal(noise(image, 123), noise(image, 123))
    assert not np.array_equal(noise(image, 123), noise(image, 124))


@pytest.mark.parametrize("channels", [3, 4])
def test_noise_is_reproducible_on_color_images(channels):
    rng = np.random.default_rng(7)
    image = rng.integers(0, 256, size=(40, 50, channels), dtype=np.uint8)

    first = fluvel.gaussian_noise(image, sigma=20, seed=5)
    second = fluvel.gaussian_noise(image, sigma=20, seed=5)

    assert first.shape == image.shape
    assert np.array_equal(first, second)
    assert not np.array_equal(first, fluvel.gaussian_noise(image, sigma=20, seed=6))


def test_noise_differs_between_calls_without_seed():
    image = make_gray()

    assert not np.array_equal(fluvel.gaussian_noise(image), fluvel.gaussian_noise(image))


def test_invalid_seed():
    image = make_gray()

    with pytest.raises(ValueError):
        fluvel.gaussian_noise(image, seed=-1)

    with pytest.raises(ValueError):
        fluvel.gaussian_noise(image, seed=2**32)

    with pytest.raises(TypeError):
        fluvel.gaussian_noise(image, seed=1.5)


#
# Parameters
#


@pytest.mark.parametrize(
    "field, value",
    [
        ("iterations", -1),
        ("kappa", 0.0),
        ("kappa", -5.0),
        ("lambda_", 0.0),
        ("lambda_", -0.1),
        ("lambda_", 0.5),
    ],
)
def test_invalid_anisotropic_diffusion_parameters(field, value):
    params = fluvel.AnisoParams()
    setattr(params, field, value)

    with pytest.raises(ValueError):
        fluvel.anisotropic_diffusion(make_gray(), params)


def test_anisotropic_diffusion_without_iteration_is_identity():
    params = fluvel.AnisoParams()
    params.iterations = 0

    gray = make_gray()

    assert np.array_equal(fluvel.anisotropic_diffusion(gray, params), gray)


@pytest.mark.parametrize(
    "field, value",
    [("Na", 0), ("Na", -1), ("Ns", 0), ("Ns", -1), ("disk_radius", 0), ("disk_radius", -2)],
)
def test_invalid_active_contour_parameters(field, value):
    params = fluvel.ActiveContourParams()
    setattr(params, field, value)

    with pytest.raises(ValueError):
        fluvel.active_contour(make_gray(), active_contour_params=params)


def test_ns_is_ignored_when_cycle2_is_disabled():
    params = fluvel.ActiveContourParams()
    params.cycle2_enabled = False
    params.Ns = 0

    fluvel.active_contour(make_gray(), active_contour_params=params)


def test_invalid_region_parameters():
    region = fluvel.RegionModelParams()
    region.lambda_in = -1

    with pytest.raises(ValueError):
        fluvel.active_contour(make_gray(), region_model_params=region)

    region = fluvel.RegionModelParams()
    region.weights = fluvel.Components3i(1, -1, 1)

    with pytest.raises(ValueError):
        fluvel.active_contour(make_bgr(), region_model_params=region)


@pytest.mark.parametrize("radius", [0, -3])
def test_invalid_radius(radius):
    with pytest.raises(ValueError):
        fluvel.mean(make_gray(), radius=radius)


#
# Contours
#


def test_find_contours_of_an_empty_mask_returns_no_contour():
    contours = fluvel.find_contours(np.zeros((20, 30), np.uint8))

    assert contours.outer.shape == (0, 2)
    assert contours.inner.shape == (0, 2)
    assert contours.mask.shape == (20, 30)
    assert not contours.mask.any()


def test_find_contours_of_a_full_mask_returns_the_image_border():
    contours = fluvel.find_contours(np.full((20, 30), 255, np.uint8))

    assert contours.outer.shape == (0, 2)
    assert contours.inner.shape == (2 * (20 + 30) - 4, 2)
    assert (contours.mask == 255).all()

    x, y = contours.inner[:, 0], contours.inner[:, 1]

    assert ((x == 0) | (x == 29) | (y == 0) | (y == 19)).all()


def test_find_contours_mask_matches_the_input_mask():
    mask = np.zeros((40, 50), np.uint8)
    mask[10:25, 15:40] = 255

    contours = fluvel.find_contours(mask)

    assert contours.outer.shape[0] > 0
    assert contours.inner.shape[0] > 0
    assert np.array_equal(contours.mask, mask)


def test_find_contours_uses_the_first_channel_of_color_masks():
    mask = np.zeros((40, 50), np.uint8)
    mask[10:25, 15:40] = 255

    color = np.dstack([mask, np.zeros_like(mask), np.zeros_like(mask)])

    assert np.array_equal(
        fluvel.find_contours(color).inner, fluvel.find_contours(mask).inner
    )


def test_active_contour_result():
    image = make_gray()

    result = fluvel.active_contour(image)

    assert (result.grid_height, result.grid_width) == image.shape
    assert result.outer.dtype == np.int32
    assert result.outer.shape[1] == 2
    assert result.mask.shape == image.shape
    assert result.mask.dtype == np.uint8
    assert set(np.unique(result.mask)) <= {0, 255}

    # The inner boundary belongs to the region.
    assert (result.mask[result.inner[:, 1], result.inner[:, 0]] == 255).all()

    # The bright disk is found.
    y, x = np.ogrid[:96, :128]
    disk = (x - 64) ** 2 + (y - 48) ** 2 < 30**2
    inside = result.mask == 255
    iou = (inside & disk).sum() / (inside | disk).sum()

    assert iou > 0.8


def test_active_contour_is_deterministic():
    image = make_bgr()

    first = fluvel.active_contour(image)
    second = fluvel.active_contour(image)

    assert np.array_equal(first.outer, second.outer)
    assert np.array_equal(first.mask, second.mask)


def test_active_contour_accepts_all_image_layouts():
    gray = make_gray()
    layouts = [gray, gray[..., None], np.dstack([gray] * 3), np.dstack([gray] * 4)]

    for image in layouts:
        assert fluvel.active_contour(image).mask.shape == gray.shape


def test_active_contour_with_an_initial_mask():
    image = make_gray()

    phi = np.zeros(image.shape, np.uint8)
    phi[20:70, 30:100] = 255

    result = fluvel.active_contour(image, phi_mask=phi)

    assert result.mask.shape == image.shape


def test_active_contour_invalid_initial_mask():
    image = make_gray()

    with pytest.raises(ValueError):
        fluvel.active_contour(image, phi_mask=np.zeros((10, 10), np.uint8))

    with pytest.raises(TypeError):
        fluvel.active_contour(image, phi_mask=np.zeros(image.shape, np.float32))

    with pytest.raises(TypeError):
        fluvel.active_contour(image, phi_mask="mask")


def test_hausdorff_metrics():
    mask = np.zeros((40, 50), np.uint8)
    mask[10:25, 15:40] = 255
    contour = fluvel.find_contours(mask).outer

    same = fluvel.hausdorff_metrics(contour, contour)

    assert same.distance == 0
    assert same.modified_distance == 0
    assert same.quantile_distance == 0
    assert same.centroids_distance == 0

    # Absolute coordinates by default: a translation is a distance.
    moved = contour + np.array([3, 0], np.int32)
    shifted = fluvel.hausdorff_metrics(contour, moved)

    assert shifted.centroids_distance == pytest.approx(3.0)
    assert 0 < shifted.distance <= 3.0 + 1e-6

    # With aligned centroids, only the shape matters.
    aligned = fluvel.hausdorff_metrics(contour, moved, align_centroids=True)

    assert aligned.distance == 0
    assert aligned.modified_distance == 0
    assert aligned.quantile_distance == 0
    assert aligned.centroids_distance == pytest.approx(3.0)

    # A different shape gives a positive distance.
    other = np.zeros((40, 50), np.uint8)
    other[5:35, 20:30] = 255

    assert fluvel.hausdorff_metrics(contour, fluvel.find_contours(other).outer).distance > 0


def reference_hausdorff(a, b, quantile, align):
    """Brute-force reference, same definitions as the library."""
    a = a.astype(np.float64)
    b = b.astype(np.float64)

    if align:
        a = a - a.mean(axis=0)
        b = b - b.mean(axis=0)

    d = np.sqrt(((a[:, None, :] - b[None, :, :]) ** 2).sum(axis=2))
    ab = np.sort(d.min(axis=1))
    ba = np.sort(d.min(axis=0))

    def q(v):
        return v[((len(v) - 1) * quantile) // 100]

    return (max(ab.max(), ba.max()), max(ab.mean(), ba.mean()), max(q(ab), q(ba)))


@pytest.mark.parametrize("align", [False, True])
@pytest.mark.parametrize("quantile", [0, 50, 80, 95, 100])
def test_hausdorff_metrics_are_exact(align, quantile):
    rng = np.random.default_rng(1234)

    for _ in range(5):
        a = rng.integers(0, 60, size=(int(rng.integers(20, 200)), 2)).astype(np.int32)
        b = rng.integers(10, 80, size=(int(rng.integers(20, 200)), 2)).astype(np.int32)

        result = fluvel.hausdorff_metrics(a, b, quantile=quantile, align_centroids=align)
        expected = reference_hausdorff(a, b, quantile, align)

        assert result.distance == pytest.approx(expected[0], rel=1e-5)
        assert result.modified_distance == pytest.approx(expected[1], rel=1e-5)
        assert result.quantile_distance == pytest.approx(expected[2], rel=1e-5)


def test_hausdorff_metrics_exact_on_contours():
    gt = np.zeros((120, 160), np.uint8)
    gt[20:100, 30:130] = 255
    other = np.zeros((120, 160), np.uint8)
    other[28:95, 38:140] = 255

    a = fluvel.find_contours(gt).outer
    b = fluvel.find_contours(other).outer

    for align in (False, True):
        result = fluvel.hausdorff_metrics(a, b, quantile=95, align_centroids=align)
        expected = reference_hausdorff(a, b, 95, align)

        assert result.distance == pytest.approx(expected[0], rel=1e-5)
        assert result.modified_distance == pytest.approx(expected[1], rel=1e-5)
        assert result.quantile_distance == pytest.approx(expected[2], rel=1e-5)


def test_hausdorff_metrics_invalid_arguments():
    contour = np.array([[0, 0], [1, 1]], dtype=np.int32)

    with pytest.raises(ValueError):
        fluvel.hausdorff_metrics(contour, contour, quantile=101)

    with pytest.raises(ValueError):
        fluvel.hausdorff_metrics(contour, np.zeros((0, 2), np.int32))

    with pytest.raises(ValueError):
        fluvel.hausdorff_metrics(contour, np.zeros((3, 3), np.int32))


def test_repr():
    result = fluvel.active_contour(make_gray())

    assert repr(result).startswith("ContourSet(outer=")
    assert "Na=30" in repr(fluvel.ActiveContourParams())
    assert "Lab" in repr(
        _with(fluvel.RegionModelParams(), color_space=fluvel.ColorSpaceOption.Lab)
    )
    assert "iterations=10" in repr(fluvel.AnisoParams())


def _with(obj, **fields):
    for name, value in fields.items():
        setattr(obj, name, value)

    return obj


#
# Threads
#


def test_the_gil_is_released_during_the_computation():
    image = np.kron(make_gray(), np.ones((6, 6), np.uint8))

    ticks = 0
    running = True

    def count():
        nonlocal ticks

        while running:
            ticks += 1
            time.sleep(0.002)

    thread = threading.Thread(target=count)
    thread.start()

    time.sleep(0.05)
    ticks = 0

    start = time.perf_counter()
    fluvel.active_contour(image)
    duration = time.perf_counter() - start

    running = False
    thread.join()

    # With the GIL held, the counting thread would be almost frozen.
    assert ticks > 0.2 * duration / 0.002
