#ifndef IMAGE_SPAN_HPP
#define IMAGE_SPAN_HPP

#include <cassert>
#include <bit>

#include "color.hpp"

namespace ofeli_ip
{
/*
class ImageSpan final
{
    ImageSpan(const unsigned char* data,
              int width, int height,
              int channels = 1,
              int stride = 0)
        : data_(data),
        width_(width), height_(height), channels_(channels)
    {
        assert( data != nullptr );
        assert( width  > 0 );
        assert( height > 0 );
        assert( channels > 0 );
        assert( stride >= 0 );

        if ( stride == 0 )
            stride_ = width_;
        else
            stride_ = stride;


        assert( stride_ > 0 );
    }

    // to do ??
    unsigned char pixel_at(int offset) const noexcept
    {
        assert( offset >= 0 && offset < size() );

        return data_[offset];
    }

    // to do ??
    unsigned char pixel_at(int x, int y) const noexcept
    {
        assert( x >= 0 && x < width_ );
        assert( y >= 0 && y < height_ );

        return data_[ x+y*width_ ];
    }

    int width()  const { return width_; }
    int height() const { return height_; }
    int size()   const { return width_ * height_; }

private:
    const unsigned char* data_;
    int widthPixels;
    int heightPixels;
    int channelsPerPixel_;  // 1 = gray, 3 = RGB, 4 = RGBA
    int strideBytes_;       // bytes per row
};
*/
class ImageSpan8 final
{

public:

    ImageSpan8(const unsigned char* img_data,
               int img_width,
               int img_height):
        data_(img_data),
        width_(img_width),
        height_(img_height)
    {
        assert( data_ != nullptr );
        assert( width_  > 0 );
        assert( height_ > 0 );
    }

    unsigned char pixel_at(int offset) const noexcept
    {
        assert( offset >= 0 && offset < size() );

        return data_[offset];
    }

    unsigned char pixel_at(int x, int y) const noexcept
    {
        assert( x >= 0 && x < width_ );
        assert( y >= 0 && y < height_ );

        return data_[ x+y*width_ ];
    }

    int width()  const { return width_; }
    int height() const { return height_; }
    int size()   const { return width_ * height_; }

private:

    const unsigned char* data_;
    int width_;
    int height_;
};

class ImageSpan32 final
{
public:
    ImageSpan32(const unsigned char* data,
                int width,
                int height):
        data_(data),
        width_(width),
        height_(height)
    {
        assert(data_ != nullptr);
        assert(width_  > 0);
        assert(height_ > 0);
    }

    // --- Accès par offset
    Bgra32 pixel_bgra_at(int offset) const noexcept
    {
        assert( offset >= 0 && offset < size() );

        const unsigned char* p = data_ + offset * 4;
        return { p[0], p[1], p[2], p[3] };
    }

    // --- Accès par coordonnées
    Bgra32 pixel_bgra_at(int x, int y) const noexcept
    {
        assert( x >= 0 && x < width_ );
        assert( y >= 0 && y < height_ );

        const unsigned char* p = data_ + (x + y * width_) * 4;
        return { p[0], p[1], p[2], p[3] };
    }

    // --- Accès logique RGB (indépendant endianness)
    Rgb_uc pixel_rgb_at(int offset) const noexcept
    {
        assert( offset >= 0 && offset < size() );

        const auto px = pixel_bgra_at(offset);
        return Rgb_uc{ px.red, px.green, px.blue };
    }

    Rgb_uc pixel_rgb_at(int x, int y) const noexcept
    {
        assert( x >= 0 && x < width_ );
        assert( y >= 0 && y < height_ );

        const auto px = pixel_bgra_at(x, y);
        return Rgb_uc{ px.red, px.green, px.blue };
    }

    int width()  const { return width_; }
    int height() const { return height_; }
    int size()   const { return width_ * height_; }

private:
    const unsigned char* data_;
    int width_;
    int height_;
};

}

#endif // IMAGE_SPAN_HPP
