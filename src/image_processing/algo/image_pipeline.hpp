// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#pragma once

#include "image_owner.hpp"
#include "image_view.hpp"
#include "processing_params.hpp"

namespace fluvel_ip
{

class ImagePipeline
{
public:
    void reset(const ImageView& input);
    void apply(const ImageView& input, const ProcessingParams& params);

    ImageView outputView() const;
    const ImageOwner& output() const;

private:
    ImageOwner bufferA_;
    ImageOwner bufferB_;

    ImageOwner* currentPtr_ = nullptr;
};

} // namespace fluvel_ip
