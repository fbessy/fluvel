#include "frame_clock.hpp"

namespace ofeli_app {

QElapsedTimer FrameClock::timer;

void FrameClock::FrameClock::init()
{
    if (!timer.isValid())
        timer.start();
}

qint64 FrameClock::FrameClock::nowNs()
{
    return timer.nsecsElapsed();
}

}
