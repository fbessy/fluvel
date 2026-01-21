#ifndef FILTERS2_HPP
#define FILTERS2_HPP

namespace ofeli_ip
{

class Filters2
{
public:
    Filters2();
};

}

#endif // FILTERS2_HPP


#if 0

qsdqsdqsd

void mean_filter(
    const ImageView& src,
    ImageView& dst,
    const MeanKernel& k)
{
    const int pad = k.radius;

    for (int y = 0; y < dst.height; ++y)
    {
        for (int x = 0; x < dst.width; ++x)
        {
            const int base =
                (x + pad) + (y + pad) * src.stride;

            unsigned char* out =
                dst.data + (x + y * dst.stride) * dst.channels;

            for (int c = 0; c < src.channels; ++c)
            {
                int sum = 0;

                const unsigned char* center =
                    src.data + (base * src.channels) + c;

                for (int off : k.offsets)
                {
                    sum += center[off * src.channels];
                }

                out[c] = static_cast<unsigned char>(
                    sum / k.area);
            }
        }
    }
}


struct MeanKernel
{
    int radius;
    int area;
    std::vector<int> offsets; // offsets en pixels
};

MeanKernel make_mean_kernel(int kernel_length, int stride)
{
    const int r = kernel_length / 2;

    MeanKernel k;
    k.radius = r;
    k.area   = kernel_length * kernel_length;
    k.offsets.reserve(k.area);

    for (int dy = -r; dy <= r; ++dy)
    {
        for (int dx = -r; dx <= r; ++dx)
        {
            k.offsets.push_back(dx + dy * stride);
        }
    }

    return k;
}

struct ImageView
{
    unsigned char* data;
    int width;
    int height;
    int stride;    // en pixels
    int channels;  // 1, 3, 4
};

4. Kernel moyenneur factorisable (pré-calcul)
Kernel 1D (clé pour le RGB propre)
struct MeanKernel1D
{
    int radius;
    int size;
    float inv_size;
};

MeanKernel1D make_mean_kernel_1d(int length)
{
    MeanKernel1D k;
    k.radius = length / 2;
    k.size   = length;
    k.inv_size = 1.0f / float(length);
    return k;
}

inline int mirror(int x, int max)
{
    if (x < 0)       return -x - 1;
    if (x >= max)    return 2*max - x - 1;
    return x;
}

void fill_padding_mirror(
    const ImageView& src,
    ImageView& dst,
    int pad)
{
    for (int y = 0; y < dst.height; ++y)
    {
        int sy = mirror(y - pad, src.height);

        for (int x = 0; x < dst.width; ++x)
        {
            int sx = mirror(x - pad, src.width);

            const unsigned char* s =
                src.data + (sx + sy * src.stride) * src.channels;

            unsigned char* d =
                dst.data + (x + y * dst.stride) * dst.channels;

            for (int c = 0; c < src.channels; ++c)
                d[c] = s[c];
        }
    }
}

3. Padding préalable (une fois)
Allocation du buffer paddé
ImageView make_padded_view(
    const ImageView& src,
    int pad,
    std::vector<unsigned char>& storage)
{
    const int pw = src.width  + 2 * pad;
    const int ph = src.height + 2 * pad;

    storage.resize(pw * ph * src.channels);

    ImageView padded;
    padded.data     = storage.data();
    padded.width    = pw;
    padded.height   = ph;
    padded.stride   = pw;
    padded.channels = src.channels;

    return padded;
}

struct ImageView
{
    unsigned char* data;
    int width;
    int height;
    int stride;   // en pixels
    int channels; // 1 = gray, 3 = RGB, 4 = RGBA
};


#endif
