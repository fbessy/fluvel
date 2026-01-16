#ifndef IMAGE_SPAN_HPP
#define IMAGE_SPAN_HPP

#include <cassert>
#include <bit>

namespace ofeli_ip
{

class ImageSpan8 final
{

public:

    ImageSpan8(const unsigned char* image_data1,
               int image_width1,
               int image_height1):
        image_data(image_data1),
        image_width(image_width1),
        image_height(image_height1)
    {
        assert( image_data != nullptr );
        assert( image_width  > 0 );
        assert( image_height > 0 );
    }

    unsigned char pixel_at(int offset) const noexcept
    {
        assert( offset >= 0 && offset < get_size() );

        return image_data[offset];
    }

    unsigned char pixel_at(int x, int y) const noexcept
    {
        assert( x >= 0 && x < image_width );
        assert( y >= 0 && y < image_height );

        return image_data[ x+y*image_width ];
    }

    int get_width()  const { return image_width; }
    int get_height() const { return image_height; }
    int get_size()   const { return image_width * image_height; }

private:

    const unsigned char* image_data;
    int image_width;
    int image_height;
};

// Colors

struct Bgra32
{
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char alpha;
};

template <typename T>
struct Rgb
{
    T red;
    T green;
    T blue;
};

using Rgb_uc = Rgb<unsigned char>;
using Rgb_i = Rgb<int>;
using Rgb_ui = Rgb<unsigned int>;

class ImageSpan32 final
{
public:
    ImageSpan32(const unsigned char* data,
                int width,
                int height):
        image_data(data),
        image_width(width),
        image_height(height)
    {
        assert(image_data != nullptr);
        assert(image_width  > 0);
        assert(image_height > 0);
    }

    // --- Accès par offset
    Bgra32 pixel_bgra_at(int offset) const noexcept
    {
        assert( offset >= 0 && offset < get_size() );

        const unsigned char* p = image_data + offset * 4;
        return { p[0], p[1], p[2], p[3] };
    }

    // --- Accès par coordonnées
    Bgra32 pixel_bgra_at(int x, int y) const noexcept
    {
        assert( x >= 0 && x < image_width );
        assert( y >= 0 && y < image_height );

        const unsigned char* p = image_data + (x + y * image_width) * 4;
        return { p[0], p[1], p[2], p[3] };
    }

    // --- Accès logique RGB (indépendant endianness)
    Rgb_uc pixel_rgb_at(int offset) const noexcept
    {
        assert( offset >= 0 && offset < get_size() );

        const auto px = pixel_bgra_at(offset);
        return { px.red, px.green, px.blue };
    }

    Rgb_uc pixel_rgb_at(int x, int y) const noexcept
    {
        assert( x >= 0 && x < image_width );
        assert( y >= 0 && y < image_height );

        const auto px = pixel_bgra_at(x, y);
        return { px.red, px.green, px.blue };
    }

    int get_width()  const { return image_width; }
    int get_height() const { return image_height; }
    int get_size()   const { return image_width * image_height; }

private:
    const unsigned char* image_data;
    int image_width;
    int image_height;
};

}

#endif // IMAGE_SPAN_HPP
