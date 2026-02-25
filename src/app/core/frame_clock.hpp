#ifndef FRAME_CLOCK_HPP
#define FRAME_CLOCK_HPP

#include <QElapsedTimer>

namespace ofeli_app
{

class FrameClock
{
public:
    static void init();
    static qint64 nowNs();

private:
    static QElapsedTimer timer;
};

} // namespace ofeli_app

#endif // FRAME_CLOCK_HPP
