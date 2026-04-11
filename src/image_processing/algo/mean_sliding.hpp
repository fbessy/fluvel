// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"

namespace fluvel_ip::filter
{

void meanSliding(const ImageView& input, ImageOwner& output, int radius);
ImageOwner meanSliding(const ImageView& input, int radius);

class MeanSliding
{
public:
    void apply(const ImageView& input, int radius);

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

private:
    void reset(const ImageView& input);

    ImageOwner buffer1_;
    ImageOwner buffer2_;

    int width_{0};
    int height_{0};
};

} // namespace fluvel_ip
