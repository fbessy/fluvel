#ifndef COMMON_SETTINGS_HPP
#define COMMON_SETTINGS_HPP

namespace ofeli_gui
{

enum SpeedModel : int
{
    REGION_BASED = 0,  // Chan-Vese model
    EDGE_BASED         // Geodesic model
};

enum Language : int
{
    SYSTEM = 0,
    ENGLISH,
    FRENCH
};

struct RgbColor
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;

    RgbColor divide(unsigned char n)
    {
        RgbColor tmp;

        tmp.red = red / n;
        tmp.green = green / n;
        tmp.blue = blue / n;

        return tmp;
    }
};

enum ComboBoxColorIndex : int
{
    RED = 0,
    GREEN,
    BLUE,
    CYAN,
    MAGENTA,
    YELLOW,
    BLACK,
    WHITE,
    SELECTED,
    NO
};

}

#endif // COMMON_SETTINGS_HPP
