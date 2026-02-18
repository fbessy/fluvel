#include "active_contour_worker.hpp"

#include "region_ac.hpp"
#include "region_color_ac.hpp"
#include "edge_ac.hpp"

#include "contour_rendering_qimage.hpp"
#include "image_span.hpp"
#include "image_adapters.hpp"
#include "algo_stats.hpp"

#include <QTimer>
#include <QElapsedTimer>
#include <QDebug>

#include <cassert>

namespace ofeli_app
{

constexpr int    workerPeriod_ms            = 16;
constexpr qint64 timeSliceInteractive_ms    = 10;
constexpr qint64 timeSliceConverge_ms       = 15;

ActiveContourWorker::ActiveContourWorker()
    : QObject(nullptr),
    state_(WorkerState::Uninitialized),
    mode_(RunMode::Interactive),
    timer_(new QTimer(this)),
    timeSlice_ms_(timeSliceInteractive_ms)
{
    timer_->setInterval(workerPeriod_ms);
    connect(timer_, &QTimer::timeout,
            this, &ActiveContourWorker::onTimeout);
}

void ActiveContourWorker::restart()
{
    if ( state_ == WorkerState::Initializing )
        return;

    setMode( RunMode::Interactive );

    const auto& fc = config_.compute.processing;

    // if there is at least one random operation,
    // preprocessing is done at each restart
    if(   fc.hasProcessing() &&
        (    fc.has_gaussian_noise
          || fc.has_salt_noise
          || fc.has_speckle_noise ) )
    {
        applyProcessing();
    }

    initializeActiveContour();

    if ( state_ != WorkerState::Ready )
        return;

    updateStats();
    emitContour();

    initialShown_ = true;

    start();
}

void ActiveContourWorker::start()
{
    if ( state_ == WorkerState::Uninitialized ||
         state_ == WorkerState::Initializing )
        return;

    setState( WorkerState::Running );
    timer_->start();
}

void ActiveContourWorker::togglePause()
{
    if ( state_ == WorkerState::Uninitialized ||
         state_ == WorkerState::Initializing )
        return;


    setMode( RunMode::Interactive );

    if (state_ == WorkerState::Running)
    {
        suspend();
    }
    else if (state_ == WorkerState::Suspended)
    {
        resume();
    }
    else if (state_ == WorkerState::Ready)
    {
        if ( !initialShown_ )
        {
            updateStats();
            emitContour();

            initialShown_ = true;
        }
        else
        {
            resume();
        }
    }
}

void ActiveContourWorker::step()
{
    if ( state_ == WorkerState::Uninitialized ||
         state_ == WorkerState::Initializing )
        return;


    setMode( RunMode::Interactive );

    if ( state_ == WorkerState::Running )
    {
        suspend();
    }
    else if ( state_ == WorkerState::Suspended )
    {
        performStep();
    }
    else if ( state_ == WorkerState::Ready )
    {
        if ( !initialShown_ )
        {
            updateStats();
            emitContour();

            initialShown_ = true;
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
        emitContour();
    }
}

void ActiveContourWorker::converge()
{
    if ( state_ == WorkerState::Uninitialized ||
         state_ == WorkerState::Initializing )
        return;


    if ( state_ == WorkerState::Ready )
    {
        updateStats();
        emitContour();

        initialShown_ = true;
    }

    setMode( RunMode::Converge );

    if ( state_ != WorkerState::Running )
        start();
}

bool ActiveContourWorker::stepOnceAlgo()
{
    assert( ac_ != nullptr );

    if ( ac_->is_stopped() )
    {
        initializeActiveContour();
    }
    else
    {
        ac_->step();
    }

    return ac_->is_stopped();
}

void ActiveContourWorker::applyPreprocess()
{
    timer_->stop();
    setState( WorkerState::Initializing );

    applyDownscale();
    applyProcessing();
}

void ActiveContourWorker::applyDownscale()
{
    if ( inputImage_.isNull() )
        return;

    if ( config_.compute.initialPhi.isNull() )
        return;

    const bool hasDownscale = config_.compute.downscale.hasDownscale;
    const int downscale_fctr = config_.compute.downscale.downscaleFactor;

    scaledPhi_ = config_.compute.initialPhi;

#ifdef OFELI_DEBUG
    int bpp = inputImage_.depth() / 8;  // ou 4 pour ARGB32
    int expected = inputImage_.width() * bpp;

    qDebug() << "bytesPerLine =" << inputImage_.bytesPerLine()
             << "expected =" << expected
             << "padding =" << inputImage_.bytesPerLine() - expected;
#endif

    if ( hasDownscale )
    {
        assert( downscale_fctr == 2 || downscale_fctr == 4 );

        downscaledImage_ = inputImage_.scaled(inputImage_.width()/downscale_fctr,
                                              inputImage_.height()/downscale_fctr,
                                              Qt::IgnoreAspectRatio,
                                              Qt::FastTransformation);

        scaledPhi_ = scaledPhi_.scaled(downscaledImage_.width(),
                                       downscaledImage_.height(),
                                       Qt::IgnoreAspectRatio,
                                       Qt::FastTransformation);
    }
    else
    {
        downscaledImage_ = inputImage_;
    }

#ifdef OFELI_DEBUG
    int bpp2 = downscaledImage_.depth() / 8;  // ou 4 pour ARGB32
    int expected2 = downscaledImage_.width() * bpp2;

    qDebug() << "bytesPerLine =" << downscaledImage_.bytesPerLine()
             << "expected =" << expected2
             << "padding =" << downscaledImage_.bytesPerLine() - expected2;
#endif
}

void ActiveContourWorker::applyProcessing()
{
    if ( downscaledImage_.isNull() )
        return;

    //float elapsed_time;
    //std::clock_t start_time, stop_time;

    QImage img;

    if ( downscaledImage_.format() == QImage::Format_Grayscale8 )
        img = downscaledImage_;
    else if ( downscaledImage_.format() == QImage::Format_RGB32 )
        img = downscaledImage_.convertToFormat( QImage::Format_RGB888 );

    if ( img.isNull() )
        return;

    ofeli_ip::Filters filters(img.constBits(),
                              img.width(), img.height(),
                              img.bytesPerLine()/img.width());

    //start_time = std::clock();

    const auto& fc = config_.compute.processing;

    if( fc.hasProcessing() )
    {
        if( fc.has_gaussian_noise )
        {
            filters.gaussian_white_noise(fc.std_noise);
        }
        if( fc.has_salt_noise )
        {
            filters.salt_pepper_noise(fc.proba_noise);
        }
        if( fc.has_speckle_noise )
        {
            filters.speckle(fc.std_speckle_noise);
        }

        if( fc.has_mean_filt )
        {
            filters.mean_filtering(fc.kernel_mean_length);
        }
        if( fc.has_gaussian_filt )
        {
            filters.gaussian_filtering(fc.kernel_gaussian_length, fc.sigma);
        }

        if( fc.has_median_filt )
        {
            if( fc.has_O1_algo )
            {
                filters.median_filtering_o1(fc.kernel_median_length);
            }
            else
            {
                filters.median_filtering_oNlogN(fc.kernel_median_length);
            }
        }
        if( fc.has_aniso_diff )
        {
            filters.anisotropic_diffusion(fc.max_itera, fc.lambda, fc.kappa, fc.aniso_option);
        }

        if( fc.has_open_filt )
        {
            if( fc.has_O1_morpho )
            {
                filters.opening_o1(fc.kernel_open_length);
            }
            else
            {
                filters.opening(fc.kernel_open_length);
            }
        }

        if( fc.has_close_filt )
        {
            if( fc.has_O1_morpho )
            {
                filters.closing_o1(fc.kernel_close_length);
            }
            else
            {
                filters.closing(fc.kernel_close_length);
            }
        }

        if( fc.has_top_hat_filt )
        {
            if( fc.is_white_top_hat )
            {
                if( fc.has_O1_morpho )
                {
                    filters.white_top_hat_o1(fc.kernel_tophat_length);
                }
                else
                {
                    filters.white_top_hat(fc.kernel_tophat_length);
                }
            }
            else
            {
                if( fc.has_O1_morpho )
                {
                    filters.black_top_hat_o1(fc.kernel_tophat_length);
                }
                else
                {
                    filters.black_top_hat(fc.kernel_tophat_length);
                }
            }
        }

        //stop_time = std::clock();
        //elapsed_time = float(stop_time - start_time) / float(CLOCKS_PER_SEC);

        if ( img.format() == QImage::Format_RGB888 )
        {
            processedImage_ = QImage(filters.get_filtered(),
                                     img.width(), img.height(),
                                     QImage::Format_RGB888).convertToFormat( QImage::Format_RGB32 );
        }
        else if ( img.format() == QImage::Format_Grayscale8 )
        {
            processedImage_ = QImage(filters.get_filtered(),
                                     img.width(), img.height(),
                                     QImage::Format_Grayscale8).copy();
        }
    }
    else
    {
        processedImage_ = downscaledImage_;
    }

    emit processedImageReady(processedImage_);

    //return elapsed_time;
}

void ActiveContourWorker::initializeFromInput(const QImage& input,
                                              const ImageSessionSettings& config)
{
    timer_->stop();
    setState( WorkerState::Initializing );

    inputImage_ = input;
    config_ = config;

    applyPreprocess();

    initializeActiveContour();

    if ( state_ != WorkerState::Ready )
        return;

    updateStats();
    emitContour();

    initialShown_ = true;
}

void ActiveContourWorker::initializeActiveContour()
{
    if( processedImage_.isNull() )
        return;

    timer_->stop();
    setState( WorkerState::Initializing );

    ac_.reset();

    ofeli_ip::ContourData initialCD(scaledPhi_.constBits(),
                                    scaledPhi_.width(),
                                    scaledPhi_.height(),
                                    config_.compute.algo.connectivity);

    bool is_rgb = ( processedImage_.format() != QImage::Format_Grayscale8 );

    const auto processedImg = image_span_from_qimage( processedImage_ );


    if( is_rgb )
    {
        ac_ = std::make_unique<ofeli_ip::RegionColorAc>(processedImg,
                                                        std::move(initialCD),
                                                        config_.compute.algo.acConfig,
                                                        config_.compute.algo.regionAcConfig);
    }
    else
    {
        ac_ = std::make_unique<ofeli_ip::RegionAc>(processedImg,
                                                   std::move(initialCD),
                                                   config_.compute.algo.acConfig,
                                                   config_.compute.algo.regionAcConfig);
    }

    setState( WorkerState::Ready );
    initialShown_ = false;
}

void ActiveContourWorker::finish()
{
    timer_->stop();
    setState( WorkerState::Finished );
}

void ActiveContourWorker::finalizeAndPrepareNextRun()
{
    finish();

    updateStats();
    emitContour();

    initializeActiveContour();
}

void ActiveContourWorker::suspend()
{
    if ( state_ == WorkerState::Running )
    {
        timer_->stop();
        setState( WorkerState::Suspended );

        updateStats();
        emitContour();
    }
}

void ActiveContourWorker::resume()
{
    if ( state_ == WorkerState::Suspended ||
         state_ == WorkerState::Ready )
        start();
}

void ActiveContourWorker::onTimeout()
{
    assert( ac_ != nullptr );
    assert( state_ == WorkerState::Running );

    QElapsedTimer timer;
    timer.start();

    while ( timer.elapsed() < timeSlice_ms_ )
    {
        if ( stepOnceAlgo() )
        {
            finalizeAndPrepareNextRun();
            return;
        }
    }

    if ( mode_ == RunMode::Interactive )
    {
        updateStats();
        emitContour();
    }
}

void ActiveContourWorker::updateStats()
{
    AlgoStats stats;
    stats.iteration = ac_ ? ac_->step_count() : 0;

    QMutexLocker lock(&statsMutex_);
    currentStats_ = stats;
}

void ActiveContourWorker::emitContour()
{
    if ( ac_ == nullptr )
        return;

    const auto& l_out = ac_->l_out();
    const auto& l_in  = ac_->l_in();

    QVector<QPoint> l_out_pts;
    QVector<QPoint> l_in_pts;

    l_out_pts.reserve(l_out.size());
    l_in_pts.reserve(l_in.size());

    for (const auto& p : l_out)
        l_out_pts.emplace_back( p.x(), p.y() );

    for (const auto& p : l_in)
        l_in_pts.emplace_back( p.x(), p.y() );

    emit contourUpdated(l_out_pts, l_in_pts);
}

AlgoStats ActiveContourWorker::currentStats() const
{
    QMutexLocker lock(&statsMutex_);
    return currentStats_;
}

void ActiveContourWorker::setMode(RunMode mode)
{
    mode_ = mode;

    if ( mode_ == RunMode::Interactive )
        timeSlice_ms_ = timeSliceInteractive_ms;
    else if ( mode_ == RunMode::Converge )
        timeSlice_ms_ = timeSliceConverge_ms;
}

void ActiveContourWorker::setState(WorkerState state)
{
    state_ = state;
    emit stateChanged(state);
}

void ActiveContourWorker::setAlgoConfig(const ImageSessionSettings& config)
{
    config_ = config;

    applyPreprocess();
    initializeActiveContour();

    if ( state_ != WorkerState::Ready )
        return;

    updateStats();
    emitContour();

    initialShown_ = true;
}

}
