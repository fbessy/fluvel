#ifndef FRAME_CLOCK_HPP
#define FRAME_CLOCK_HPP

#include <QElapsedTimer>

namespace ofeli_gui {

class FrameClock
{
public:
    static void init();
    static qint64 nowNs();

private:
    static QElapsedTimer timer;
};

}

#endif // FRAME_CLOCK_HPP
