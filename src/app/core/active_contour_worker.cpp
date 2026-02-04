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

constexpr int    workerPeriod_ms            = 16;
constexpr qint64 timeSliceInteractive_ms    = 10;
constexpr qint64 timeSliceConverge_ms       = 15;

ActiveContourWorker::ActiveContourWorker()
    : QObject(nullptr),
    m_timer(new QTimer(this)),
    m_state(WorkerState::Uninitialized),
    m_mode(RunMode::Interactive),
    timeSlice_ms(timeSliceInteractive_ms)
{
    m_timer->setInterval(workerPeriod_ms);
    connect(m_timer, &QTimer::timeout,
            this, &ActiveContourWorker::onTimeout);

    QObject::connect(
        &AppSettings::instance(),
        &ApplicationSettings::imgSettingsApplied,
        this,
        &ActiveContourWorker::reloadSettings,
        Qt::QueuedConnection
        );
}

void ActiveContourWorker::restart()
{
    if ( m_state == WorkerState::Initializing )
        return;

    setMode( RunMode::Interactive );

    initializeActiveContour();

    if ( m_state != WorkerState::Ready )
        return;

    updateStats();
    emitContourOnly();

    initialShown = true;

    start();
}

void ActiveContourWorker::start()
{
    if ( m_state == WorkerState::Uninitialized ||
         m_state == WorkerState::Initializing )
        return;

    setState( WorkerState::Running );
    m_timer->start();
}

void ActiveContourWorker::togglePause()
{
    if ( m_state == WorkerState::Uninitialized ||
         m_state == WorkerState::Initializing )
        return;


    setMode( RunMode::Interactive );

    if (m_state == WorkerState::Running)
    {
        suspend();
    }
    else if (m_state == WorkerState::Suspended)
    {
        resume();
    }
    else if (m_state == WorkerState::Ready)
    {
        if ( !initialShown )
        {
            updateStats();
            emitContourOnly();

            initialShown = true;
        }
        else
        {
            resume();
        }
    }
}

void ActiveContourWorker::step()
{
    if ( m_state == WorkerState::Uninitialized ||
         m_state == WorkerState::Initializing )
        return;


    setMode( RunMode::Interactive );

    if ( m_state == WorkerState::Running )
    {
        suspend();
    }
    else if ( m_state == WorkerState::Suspended )
    {
        performStep();
    }
    else if ( m_state == WorkerState::Ready )
    {
        if ( !initialShown )
        {
            updateStats();
            emitContourOnly();

            initialShown = true;
        }
        else
        {
            performStep();
        }
    }
}

void ActiveContourWorker::performStep()
{
    if ( stepOnceAlgo() )
    {
        finalizeAndPrepareNextRun();
    }
    else
    {
        updateStats();
        emitContourOnly();
    }
}

void ActiveContourWorker::converge()
{
    if ( m_state == WorkerState::Uninitialized ||
         m_state == WorkerState::Initializing )
        return;


    if ( m_state == WorkerState::Ready )
    {
        updateStats();
        emitContourOnly();

        initialShown = true;
    }

    setMode( RunMode::Converge );

    if ( m_state != WorkerState::Running )
        start();
}

bool ActiveContourWorker::stepOnceAlgo()
{
    assert( ac != nullptr );

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

void ActiveContourWorker::initializeFromImage(const QImage& img)
{
    m_timer->stop();
    setState( WorkerState::Initializing );

    m_workImage = img;

    initializeActiveContour();

    updateStats();
    emitContourOnly();

    initialShown = true;
}

void ActiveContourWorker::finish()
{
    m_timer->stop();
    setState( WorkerState::Finished );
}

void ActiveContourWorker::finalizeAndPrepareNextRun()
{
    finish();

    updateStats();
    emitContourOnly();

    initializeActiveContour();
}

void ActiveContourWorker::suspend()
{
    if ( m_state == WorkerState::Running )
    {
        m_timer->stop();
        setState( WorkerState::Suspended );

        updateStats();
        emitContourOnly();
    }
}

void ActiveContourWorker::resume()
{
    if ( m_state == WorkerState::Suspended ||
         m_state == WorkerState::Ready )
        start();
}

void ActiveContourWorker::onTimeout()
{
    assert( ac != nullptr );
    assert( m_state == WorkerState::Running );

    QElapsedTimer timer;
    timer.start();

    while ( timer.elapsed() < timeSlice_ms )
    {
        if ( stepOnceAlgo() )
        {
            finalizeAndPrepareNextRun();
            return;
        }
    }

    if ( m_mode == RunMode::Interactive )
    {
        updateStats();
        emitContourOnly();
    }
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

    m_timer->stop();
    setState( WorkerState::Initializing );

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

    setState( WorkerState::Ready );
    initialShown = false;
}

void ActiveContourWorker::drawAndEmitResult()
{
    if( m_workImage.isNull() )
        return;

    QImage result = m_workImage.convertToFormat(QImage::Format_RGB32);

    const auto& config = AppSettings::instance().imgSessSettings.img_disp_conf;

    if ( ac == nullptr )
        return;

    const auto& l_out = ac->l_out();
    const auto& l_in  = ac->l_in();

    if ( config.display_l_out )
    {
        draw_list_to_img(l_out, config.l_out_color,
                         result);
    }

    if ( config.display_l_in )
    {
        draw_list_to_img(l_in, config.l_in_color,
                         result);
    }

    emit resultReady(result);
}

void ActiveContourWorker::emitContourOnly()
{
    // to test the former display
    //drawAndEmitResult();
        //return;

    if ( ac == nullptr )
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

void ActiveContourWorker::setMode(RunMode mode)
{
    m_mode = mode;

    if ( m_mode == RunMode::Interactive )
        timeSlice_ms = timeSliceInteractive_ms;
    else if ( m_mode == RunMode::Converge )
        timeSlice_ms = timeSliceConverge_ms;
}

void ActiveContourWorker::setState(WorkerState state)
{
    m_state = state;
    emit stateChanged(state);
}

void ActiveContourWorker::reloadSettings()
{
}

}
