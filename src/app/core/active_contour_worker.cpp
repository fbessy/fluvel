#include "active_contour_worker.hpp"

#include "region_ac.hpp"
#include "region_color_ac.hpp"
#include "edge_ac.hpp"

#include "application_settings.hpp"
#include "contour_rendering.hpp"
#include "image_span.hpp"
#include "image_adapters.hpp"

#include <QTimer>

namespace ofeli_app
{

ActiveContourWorker::ActiveContourWorker()
    : QObject(nullptr), m_timer(new QTimer(this))
{
    m_timer->setInterval(1);
    connect(m_timer, &QTimer::timeout,
            this, &ActiveContourWorker::onTimeout);
}

void ActiveContourWorker::setImage(const QImage& img)
{
    m_workImage = img;
    m_state = WorkerState::Idle;
}

void ActiveContourWorker::start()
{
    if (m_state != WorkerState::Idle &&
        m_state != WorkerState::Stopped)
        return;

    restart();
}

void ActiveContourWorker::restart()
{
    if (m_state == WorkerState::Restarting)
        return;

    m_state = WorkerState::Restarting;

    m_timer->stop();
    ac.reset();
    initializeActiveContour();

    m_state = WorkerState::Running;
    m_timer->start();
}

void ActiveContourWorker::stop()
{
    if (m_state != WorkerState::Running)
        return;

    m_timer->stop();
    m_state = WorkerState::Stopped;
}

void ActiveContourWorker::onTimeout()
{
    if (m_state != WorkerState::Running || !ac )
        return;

    if (ac->get_state() == ofeli_ip::State::STOPPED)
    {
        m_timer->stop();
        m_state = WorkerState::Stopped;
        return;
    }

    ac->evolve_one_iteration();
    //drawAndEmitResult();
    emitContourOnly();
}

void ActiveContourWorker::initializeActiveContour()
{
    if( !m_workImage.isNull() && !m_workImage.isNull() )
    {
        const auto& config = AppSettings::instance();

        ofeli_ip::ContourData initial_tmp_ctr(config.initialPhi.bits(),
                                              config.initialPhi.width(),
                                              config.initialPhi.height());

        bool is_rgb = ( m_workImage.format() != QImage::Format_Grayscale8 );

        if( config.speed == SpeedModel::REGION_BASED )
        {
            if( is_rgb )
            {
                ofeli_ip::ImageSpan32 image_rgb(image_span_32_from_qimage(m_workImage));

                ac = std::make_unique<ofeli_ip::RegionColorAc>(image_rgb,
                                                               std::move(initial_tmp_ctr),
                                                               config.algo_config,
                                                               config.region_ac_config);
            }
            else
            {
                ofeli_ip::ImageSpan8  image_grayscale(image_span_8_from_qimage(m_workImage));

                ac = std::make_unique<ofeli_ip::RegionAc>(image_grayscale,
                                                          std::move(initial_tmp_ctr),
                                                          config.algo_config,
                                                          config.region_ac_config);
            }
        }
        else if( config.speed == SpeedModel::EDGE_BASED )
        {
            ofeli_ip::ImageSpan8  image_grayscale(image_span_8_from_qimage(m_workImage));

            ac = std::make_unique<ofeli_ip::EdgeAc>(image_grayscale,
                                                    std::move(initial_tmp_ctr),
                                                    config.algo_config);
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

    ofeli_ip::ContourList l_out = ac->get_l_out();
    ofeli_ip::ContourList l_in  = ac->get_l_in();

    draw_list_to_img(l_out, config.color_out, config.outside_combo,
                     result.bits(), result.width(), result.height());
    draw_list_to_img(l_in, config.color_in, config.inside_combo,
                     result.bits(), result.width(), result.height());

    emit resultReady(result);
}

void ActiveContourWorker::emitContourOnly()
{
    if (!ac)
        return;

    int y;

    const auto& l_out = ac->get_l_out();
    const auto& l_in  = ac->get_l_in();

    QVector<QPoint> outPts;
    QVector<QPoint> inPts;

    outPts.reserve(l_out.size());
    inPts.reserve(l_in.size());

    const int width = ac->get_phi().get_width();

    for (const auto& p : l_out)
    {
        y = p.get_offset() / width;
        outPts.emplace_back(p.get_x(), y);
    }


    for (const auto& p : l_in)
    {
        y = p.get_offset() / width;
        inPts.emplace_back(p.get_x(), y);
    }

    emit contourUpdated(outPts, inPts);
}

}
