#include "active_contour_worker.hpp"

#include "region_ac.hpp"
#include "region_color_ac.hpp"
#include "edge_ac.hpp"

#include "application_settings.hpp"
#include "contour_rendering.hpp"
#include "image_span.hpp"
#include "image_adapters.hpp"
#include "algo_stats.hpp"

#include <QTimer>
#include <QElapsedTimer>

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
    if (m_state == WorkerState::Restarting)
        return;

    setState( WorkerState::Restarting );

    m_timer->stop();
    ac.reset();
    initializeActiveContour();

    setState( WorkerState::Running );
    m_timer->start();
}

void ActiveContourWorker::togglePause()
{
    if (m_state == WorkerState::Running)
        suspend();
    else if (m_state == WorkerState::Idle)
        resume();
}

void ActiveContourWorker::step()
{
    if ( m_state == WorkerState::Running )
        suspend();

    if ( m_state == WorkerState::Idle )
    {
        if ( stepOnce() )
        {
            setState( WorkerState::Stopped );
            emit finished();
        }

        updateStats();
        emitContourOnly();
    }
}

bool ActiveContourWorker::stepOnce()
{
    if ( !ac || ac->is_stopped() )
        return true;

    ac->step();

    return ac->is_stopped();
}

void ActiveContourWorker::setImage(const QImage& img)
{
    m_workImage = img.copy();
    setState( WorkerState::Idle );
}

void ActiveContourWorker::start()
{
    if (m_state != WorkerState::Idle &&
        m_state != WorkerState::Stopped)
        return;

    restart();
}

void ActiveContourWorker::stop()
{
    if (m_state != WorkerState::Running)
        return;

    m_timer->stop();
    setState( WorkerState::Stopped );
}

void ActiveContourWorker::suspend()
{
    if (m_state == WorkerState::Running)
    {
        m_timer->stop();
        setState( WorkerState::Idle );
    }
}

void ActiveContourWorker::resume()
{
    if (m_state == WorkerState::Idle)
    {
        m_timer->start();
        setState( WorkerState::Running );
    }
}

void ActiveContourWorker::onTimeout()
{
    if (m_state != WorkerState::Running || !ac)
        return;

    constexpr qint64 budgetMs = 10;
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < budgetMs)
    {
        if ( stepOnce() )
        {
            m_timer->stop();
            m_state = WorkerState::Stopped;
            emit finished();

            updateStats();
            emitContourOnly();

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
    if( !m_workImage.isNull() )
    {
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
}

void ActiveContourWorker::drawAndEmitResult()
{
    if( m_workImage.isNull() )
        return;

    QImage result = m_workImage.convertToFormat(QImage::Format_RGB32);

    const auto& config = AppSettings::instance();

    if ( ac == nullptr )
        return;

    const auto& l_out = ac->l_out_raw();
    const auto& l_in  = ac->l_in_raw();

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

    const auto& l_out = ac->l_out_raw();
    const auto& l_in  = ac->l_in_raw();

    QVector<QPoint> outPts;
    QVector<QPoint> inPts;

    outPts.reserve(l_out.size());
    inPts.reserve(l_in.size());

    for (const auto& p : l_out)
        outPts.emplace_back( p.x, p.y );

    for (const auto& p : l_in)
        inPts.emplace_back( p.x, p.y );


    emit contourUpdated(outPts, inPts);
}

AlgoStats ActiveContourWorker::currentStats() const
{
    QMutexLocker lock(&m_statsMutex);
    return m_currentStats;
}

}
