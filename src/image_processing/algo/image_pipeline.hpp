// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"
#include "processing_params.hpp"

namespace fluvel_ip
{

/**
 * @brief Image processing pipeline with reusable buffers.
 *
 * This class applies a sequence of image processing operations
 * defined by ProcessingParams.
 *
 * It uses a double-buffer strategy to avoid unnecessary allocations
 * and to efficiently chain multiple filters.
 *
 * Typical usage:
 * @code
 * ImagePipeline pipeline;
 * pipeline.reset(input);
 * pipeline.apply(input, params);
 * auto result = pipeline.outputView();
 * @endcode
 */
class ImagePipeline
{
public:
    /**
     * @brief Initialize internal buffers based on input image.
     *
     * Allocates or resizes internal buffers to match the input layout.
     *
     * @param input Input image.
     */
    void reset(const ImageView& input);

    /**
     * @brief Apply processing pipeline to an input image.
     *
     * The sequence of operations is controlled by @p params.
     *
     * @param input Input image.
     * @param params Processing parameters.
     */
    void apply(const ImageView& input, const ProcessingParams& params);

    /**
     * @brief Get a non-owning view of the output image.
     *
     * @return ImageView referencing the internal buffer.
     *
     * @warning The returned view is valid only while the pipeline object exists.
     */
    ImageView outputView() const;

    /**
     * @brief Get the output image with ownership.
     *
     * @return Reference to the internal ImageOwner.
     */
    const ImageOwner& output() const;

private:
    ImageOwner bufferA_; ///< First internal buffer.
    ImageOwner bufferB_; ///< Second internal buffer.

    /**
     * @brief Pointer to the current active buffer.
     *
     * This pointer alternates between bufferA_ and bufferB_
     * during pipeline execution (ping-pong buffering).
     */
    ImageOwner* currentPtr_ = nullptr;
};

} // namespace fluvel_ip
