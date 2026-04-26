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

/**
 * @brief Conduction function used in anisotropic diffusion.
 *
 * Defines how gradients influence diffusion strength.
 */
enum class ConductionFunction
{
    Exponential, ///< exp(-(∇I/kappa)^2), preserves strong edges.
    Rational     ///< 1 / (1 + (∇I/kappa)^2), smoother transition.
};

struct AnisoParams
{
    //!< Default conduction model.
    static constexpr ConductionFunction kDefaultConduction = {ConductionFunction::Exponential};

    //!< Default number of iterations.
    static constexpr int kDefaultIterations{10};

    //!< Default time step (stable value).
    static constexpr double kDefaultLambda{1.0 / 7.0};

    //!< Default conduction coefficient.
    static constexpr double kDefaultKappa{30.0};

    /// Conduction function model.
    ConductionFunction conduction{kDefaultConduction};

    /// Number of diffusion iterations.
    int iterations{kDefaultIterations};

    /// Time step (stability parameter).
    double lambda{kDefaultLambda};

    /// Edge threshold parameter.
    double kappa{kDefaultKappa};
};

/**
 * @brief Apply anisotropic diffusion using a preallocated output buffer.
 *
 * This version avoids allocating a new image and writes the result into @p output.
 * If the output size or format does not match the input, it will be reallocated.
 *
 * @param input Input image view.
 * @param output Output image owner (may be reused).
 * @param iterations Number of iterations.
 * @param lambda Time step controlling diffusion speed (stability parameter).
 * @param kappa Conduction coefficient controlling edge sensitivity.
 * @param conduction Conduction function model.
 *
 * @note Recommended for pipelines where memory reuse is important.
 */
void anisotropicDiffusion(const ImageView& input, ImageOwner& output,
                          const AnisoParams& params = AnisoParams{});

/**
 * @brief Apply Perona-Malik anisotropic diffusion and return a new image.
 *
 * This function smooths homogeneous regions while preserving edges
 * using a conduction function driven by local gradients.
 *
 * @param input Input image view (grayscale or multi-channel).
 * @param iterations Number of diffusion iterations.
 * @param lambda Time step (stability parameter).
 * @param kappa Conduction coefficient controlling edge sensitivity.
 * @param conduction Conduction function model.
 *
 * @return A new ImageOwner containing the filtered image.
 *
 * @note Convenience wrapper. For repeated calls, prefer the class version
 *       to reuse internal buffers and improve performance.
 */
ImageOwner anisotropicDiffusion(const ImageView& input, const AnisoParams& params = AnisoParams{});

/**
 * @brief Stateful implementation of Perona-Malik anisotropic diffusion.
 *
 * This class enables buffer reuse across multiple executions, avoiding
 * repeated memory allocations. It is particularly suitable for batch
 * processing or real-time pipelines.
 *
 * Typical usage:
 * @code
 * AnisotropicDiffusion diff;
 * diff.reset(input);
 * diff.apply(10, 1.0/7.0, 30.0, ConductionFunction::Exponential);
 * auto result = diff.outputView();
 * @endcode
 *
 * @note Internally, the input image is converted to double precision
 *       and processed in padded buffers.
 */
class AnisotropicDiffusion
{
public:
    /**
     * @brief Initializes internal buffers from an input image.
     *
     * Prepares padded buffers and copies input data.
     * Must be called before apply().
     *
     * @param input Input image view.
     */
    void reset(const ImageView& input);

    /**
     * @brief Runs anisotropic diffusion iterations.
     *
     * Applies the diffusion process using the internal buffers initialized by reset().
     *
     * @param iterations Number of iterations.
     * @param lambda Time step controlling diffusion speed.
     * @param kappa Conduction coefficient controlling edge sensitivity.
     * @param conduction Conduction function model.
     *
     * @note This function does not perform memory allocation.
     */
    void apply(const AnisoParams& params = AnisoParams{});

    /**
     * @brief Returns a non-owning view of the result image.
     *
     * @return ImageView referencing the internal output buffer.
     *
     * @warning The returned view is only valid while the object exists.
     */
    ImageView outputView() const;

    /**
     * @brief Returns the result image (const access).
     *
     * @return Reference to the internal ImageOwner.
     */
    const ImageOwner& output() const;

    /**
     * @brief Returns the result image (mutable access).
     *
     * @return Reference to the internal ImageOwner.
     */
    ImageOwner& outputRef();

private:
    /**
     * @brief Initializes internal buffers from input data.
     */
    void initFromInput(const ImageView& input);

    /**
     * @brief Pads borders to simplify neighborhood access.
     */
    void padBorders();

    /**
     * @brief Computes linear index in padded buffer.
     *
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param c Channel index.
     * @return Linear index in padded storage.
     */
    int idx(int x, int y, int c) const
    {
        return ((x + 1) + (y + 1) * stridePad_) * activeChannels_ + c;
    }

    int w_ = 0;              //!< Image width.
    int h_ = 0;              //!< Image height.
    int channels_ = 0;       //!< Number of input channels.
    int activeChannels_ = 0; //!< Number of processed channels.
    int stridePad_ = 0;      //!< Stride of padded buffers.

    std::vector<double> current_; //!< Current diffusion buffer.
    std::vector<double> next_;    //!< Next diffusion buffer.

    ImageOwner output_; //!< Output image storage.
};

} // namespace fluvel_ip::filter
