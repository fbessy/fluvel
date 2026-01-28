#include "active_contour_worker.hpp"

#include "region_ac.hpp"
#include "region_color_ac.hpp"
#include "edge_ac.hpp"

#include "application_settings.hpp"
#include "contour_rendering_qimage.hpp"
#include "image_span.hpp"
#include "image_adapters.hpp"
#include "algo_stats.hpp"

#include <QTimer>
#include <QElapsedTimer>

#include <cassert>

namespace ofeli_app
{

ActiveContourWorker::ActiveContourWorker()
    : QObject(nullptr), m_timer(new QTimer(this))
{
    m_timer->setInterval(16);
    connect(m_timer, &QTimer::timeout,
            this, &ActiveContourWorker::onTimeout);
}

void ActiveContourWorker::restart()
{
    if ( !ac )
        return;

    if ( m_state == WorkerState::Restarting )
        return;

    setState( WorkerState::Restarting );

    m_timer->stop();

    if ( !ac->is_initial() )
    {
        initializeActiveContour();
    }

    updateStats();
    emitContourOnly();

    start();
}

void ActiveContourWorker::start()
{
    setState( WorkerState::Running );
    m_timer->start();
}

void ActiveContourWorker::togglePause()
{
    if ( !ac )
        return;

    if (m_state == WorkerState::Running)
    {
        suspend();
    }
    else if (m_state == WorkerState::Idle)
    {
        resume();
    }
    else if (m_state == WorkerState::Stopped)
    {
        restart();
    }
}

void ActiveContourWorker::step()
{
    if ( !ac )
        return;

    if ( m_state == WorkerState::Running )
    {
        suspend();
    }
    else if ( m_state == WorkerState::Idle )
    {
        if ( stepOnce() )
        {
            stop();
        }

        updateStats();
        emitContourOnly();
    }
    else if ( m_state == WorkerState::Stopped )
    {
        setState( WorkerState::Idle );
        initializeActiveContour();

        updateStats();
        emitContourOnly();
    }
}

bool ActiveContourWorker::stepOnce()
{
    assert( ac );

    if ( ac->is_stopped() )
    {
        initializeActiveContour();
    }
    else
    {
        ac->step();
    }

    return ac->is_stopped();
}

void ActiveContourWorker::setImage(const QImage& img)
{
    m_timer->stop();

    m_workImage = img;

    setState( WorkerState::Idle );
    initializeActiveContour();

    updateStats();
    emitContourOnly();
}

void ActiveContourWorker::stop()
{
    m_timer->stop();
    setState( WorkerState::Stopped );

    updateStats();
    emitContourOnly();

    emit finished();

    initializeActiveContour();
}

void ActiveContourWorker::suspend()
{
    if (m_state == WorkerState::Running)
    {
        m_timer->stop();
        setState( WorkerState::Idle );

        updateStats();
        emitContourOnly();
    }
}

void ActiveContourWorker::resume()
{
    if (m_state == WorkerState::Idle)
    {
        setState( WorkerState::Running );
        m_timer->start();
    }
}

void ActiveContourWorker::onTimeout()
{
    assert( ac );
    assert( m_state == WorkerState::Running );

    constexpr qint64 budgetMs = 10;
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < budgetMs)
    {
        if ( stepOnce() )
        {
            stop();
            return;
        }
    }

    updateStats();
    emitContourOnly();
}

void ActiveContourWorker::updateStats()
{
    AlgoStats stats;
    stats.iteration = ac ? ac->step_count() : 0;

    QMutexLocker lock(&m_statsMutex);
    m_currentStats = stats;
}

void ActiveContourWorker::initializeActiveContour()
{
    if( m_workImage.isNull() )
        return;


    ac.reset();

    const auto& config = AppSettings::instance();
    unsigned int downscale_fctr = config.downscale_factor;

    workAlgo = m_workImage;
    initialPhi = config.initialPhi.copy();

#ifdef OFELI_DEBUG
    int bpp = m_workImage.depth() / 8;  // ou 4 pour ARGB32
    int expected = m_workImage.width() * bpp;

    qDebug() << "bytesPerLine =" << m_workImage.bytesPerLine()
             << "expected =" << expected
             << "padding =" << m_workImage.bytesPerLine() - expected;
#endif

    //downscale_fctr = 1;
    if ( downscale_fctr >= 2 )
    {
        workAlgo = workAlgo.scaled(workAlgo.width()/downscale_fctr,
                                   workAlgo.height()/downscale_fctr,
                                   Qt::IgnoreAspectRatio,
                                   Qt::FastTransformation);

        initialPhi = initialPhi.scaled(workAlgo.width(),
                                       workAlgo.height(),
                                       Qt::IgnoreAspectRatio,
                                       Qt::FastTransformation);
    }

#ifdef OFELI_DEBUG
    int bpp2 = workAlgo.depth() / 8;  // ou 4 pour ARGB32
    int expected2 = workAlgo.width() * bpp2;

    qDebug() << "bytesPerLine =" << workAlgo.bytesPerLine()
             << "expected =" << expected2
             << "padding =" << workAlgo.bytesPerLine() - expected2;
#endif

    ofeli_ip::ContourData initialCD(initialPhi.constBits(),
                                    initialPhi.width(),
                                    initialPhi.height(),
                                    config.connectivity);

    bool is_rgb = ( workAlgo.format() != QImage::Format_Grayscale8 );

    const auto img = image_span_from_qimage( workAlgo );

    if( config.speed == SpeedModel::REGION_BASED )
    {
        if( is_rgb )
        {
            ac = std::make_unique<ofeli_ip::RegionColorAc>(img,
                                                           std::move(initialCD),
                                                           config.algo_config,
                                                           config.region_ac_config);
        }
        else
        {
            ac = std::make_unique<ofeli_ip::RegionAc>(img,
                                                      std::move(initialCD),
                                                      config.algo_config,
                                                      config.region_ac_config);
        }
    }
}

void ActiveContourWorker::drawAndEmitResult()
{
    if( m_workImage.isNull() )
        return;

    QImage result = m_workImage.convertToFormat(QImage::Format_RGB32);

    const auto& config = AppSettings::instance();

    if ( ac == nullptr )
        return;

    const auto& l_out = ac->l_out();
    const auto& l_in  = ac->l_in();

    draw_list_to_img(l_out, config.color_out, config.outside_combo,
                     result);

    draw_list_to_img(l_in, config.color_in, config.inside_combo,
                     result);

    emit resultReady(result);
}

void ActiveContourWorker::emitContourOnly()
{
    // to test the former display
    //drawAndEmitResult();
        //return;

    if (!ac)
        return;

    const auto& l_out = ac->l_out();
    const auto& l_in  = ac->l_in();

    QVector<QPoint> outPts;
    QVector<QPoint> inPts;

    outPts.reserve(l_out.size());
    inPts.reserve(l_in.size());

    for (const auto& p : l_out)
        outPts.emplace_back( p.x(), p.y() );

    for (const auto& p : l_in)
        inPts.emplace_back( p.x(), p.y() );

    emit contourUpdated(outPts, inPts);
}

AlgoStats ActiveContourWorker::currentStats() const
{
    QMutexLocker lock(&m_statsMutex);
    return m_currentStats;
}

}
