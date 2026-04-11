// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

/**
 * @file anisotropic_diffusion.hpp
 * @brief Perona-Malik anisotropic diffusion filter implementation.
 */

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

#include <vector>

namespace fluvel_ip::filter
{

enum class ConductionFunction
{
    Exponential, ///< exp(-(∇I/kappa)^2)
    Rational     ///< 1 / (1 + (∇I/kappa)^2)
};

inline constexpr ConductionFunction kDefaultConductionFunction = ConductionFunction::Exponential;
inline constexpr int kDefaultIterations = 10;
inline constexpr double kDefaultLambda = 1.0 / 7.0;
inline constexpr double kDefaultKappa = 30.0;

/**
 * @brief Apply anisotropic diffusion using a preallocated output buffer.
 *
 * This version avoids allocating a new image and writes the result into @p output.
 * If the output size or format does not match the input, it will be reallocated.
 *
 * @param input Input image view.
 * @param output Output image owner (may be reused).
 * @param iterations Number of iterations.
 * @param lambda Time step.
 * @param kappa Conduction coefficient.
 * @param conduction Conduction function model.
 *
 * @note This function is suitable for pipelines where memory reuse is desired.
 */
void anisotropicDiffusion(const ImageView& input, ImageOwner& output,
                          int iterations = kDefaultIterations, double lambda = kDefaultLambda,
                          double kappa = kDefaultKappa,
                          ConductionFunction conduction = kDefaultConductionFunction);

/**
 * @brief Apply Perona-Malik anisotropic diffusion on an image.
 *
 * This function performs anisotropic diffusion using either exponential or
 * rational conduction functions. It smooths homogeneous regions while preserving edges.
 *
 * A new image is allocated and returned.
 *
 * @param input Input image view (grayscale or multi-channel).
 * @param iterations Number of diffusion iterations.
 * @param lambda Time step (stability parameter).
 * @param kappa Conduction coefficient controlling sensitivity to edges.
 * @param conduction Conduction function model (Exponential or Rational).
 *
 * @return A new ImageOwner containing the filtered image.
 *
 * @note This is a convenience wrapper. For better performance in repeated calls,
 *       prefer using the AnisotropicDiffusion class to reuse internal buffers.
 */
ImageOwner anisotropicDiffusion(const ImageView& input, int iterations = kDefaultIterations,
                                double lambda = kDefaultLambda, double kappa = kDefaultKappa,
                                ConductionFunction conduction = kDefaultConductionFunction);

/**
 * @brief Stateful implementation of anisotropic diffusion (Perona-Malik).
 *
 * This class allows reusing internal buffers across multiple calls, avoiding
 * repeated memory allocations. It is suitable for batch processing or repeated
 * filtering of images with the same size and format.
 *
 * Typical usage:
 * @code
 * AnisotropicDiffusion diff;
 * diff.reset(input);
 * diff.apply(10, 1.0/7.0, 30.0, ConductionFunction::Exponential);
 * auto result = diff.outputView();
 * @endcode
 *
 * @note The input image is copied into an internal double-precision buffer.
 */
class AnisotropicDiffusion
{
public:
    /**
     * @brief Initialize the internal buffers from an input image.
     *      * This function prepares internal padded buffers and copies the input data.
     * It must be called before apply().
     *      * @param input Input image view.
     */
    void reset(const ImageView& input);

    /**
     * @brief Run anisotropic diffusion iterations.
     *      * Performs the diffusion process on the internal buffers initialized by reset().
     *      * @param iterations Number of iterations.
     * @param lambda Time step controlling diffusion speed.
     * @param kappa Edge threshold controlling conduction.
     * @param conduction Conduction function model.
     *      * @note This function does not reallocate memory.
     */
    void apply(int iterations = kDefaultIterations, double lambda = kDefaultLambda,
               double kappa = kDefaultKappa,
               ConductionFunction conduction = kDefaultConductionFunction);

    /**
     * @brief Get a non-owning view of the result image.
     *      * @return ImageView referencing the internal output buffer.
     *      * @warning The returned view is valid as long as the object is alive.
     */
    ImageView outputView() const;

    /**
     * @brief Get the result image with ownership.
     *      * @return Reference to the internal ImageOwner.
     */
    const ImageOwner& output() const;

    /**
     * @brief Get the result image with ownership.
     *      * @return Reference to the internal ImageOwner.
     */
    ImageOwner& outputRef();

private:
    void initFromInput(const ImageView& input);
    void padBorders();

    int idx(int x, int y, int c) const
    {
        return ((x + 1) + (y + 1) * stridePad_) * activeChannels_ + c;
    }

    int w_ = 0;
    int h_ = 0;
    int channels_ = 0;
    int activeChannels_ = 0;
    int stridePad_ = 0;

    std::vector<double> current_;
    std::vector<double> next_;

    ImageOwner output_;
};

} // namespace fluvel_ip::filter
