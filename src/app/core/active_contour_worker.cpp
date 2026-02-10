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
    timer_(new QTimer(this)),
    state_(WorkerState::Uninitialized),
    mode_(RunMode::Interactive),
    timeSlice_ms_(timeSliceInteractive_ms),
    config_(AppSettings::instance().imgSessSettings)
{
    timer_->setInterval(workerPeriod_ms);
    connect(timer_, &QTimer::timeout,
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
    if ( state_ == WorkerState::Initializing )
        return;

    setMode( RunMode::Interactive );

    const bool has_preprocess = config_.has_preprocess;
    const auto& fc = config_.filtering_conf;

    // if there is at least one random operation,
    // preprocessing is done at each restart
    if(   has_preprocess &&
        (    fc.has_gaussian_noise
          || fc.has_salt_noise
          || fc.has_speckle_noise ) )
    {
        applyPreprocessing();
    }

    initializeActiveContour();

    if ( state_ != WorkerState::Ready )
        return;

    updateStats();
    emitContourOnly();

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
            emitContourOnly();

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
            emitContourOnly();

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
        emitContourOnly();
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
        emitContourOnly();

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

void ActiveContourWorker::processImage()
{
    timer_->stop();
    setState( WorkerState::Initializing );

    downscaleInputImage();
    applyPreprocessing();
}

void ActiveContourWorker::downscaleInputImage()
{
    const bool has_downscale = config_.downscale_conf.has_downscale;
    const int downscale_fctr = config_.downscale_conf.downscale_factor;

    initialPhi_ = config_.initial_phi.copy();

#ifdef OFELI_DEBUG
    int bpp = inputImage_.depth() / 8;  // ou 4 pour ARGB32
    int expected = inputImage_.width() * bpp;

    qDebug() << "bytesPerLine =" << inputImage_.bytesPerLine()
             << "expected =" << expected
             << "padding =" << inputImage_.bytesPerLine() - expected;
#endif

    if ( has_downscale )
    {
        assert( downscale_fctr == 2 || downscale_fctr == 4 );

        downscaledImage_ = inputImage_.scaled(inputImage_.width()/downscale_fctr,
                                              inputImage_.height()/downscale_fctr,
                                              Qt::IgnoreAspectRatio,
                                              Qt::FastTransformation);

        initialPhi_ = initialPhi_.scaled(downscaledImage_.width(),
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

void ActiveContourWorker::applyPreprocessing()
{
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

    const bool has_preprocess = config_.has_preprocess;
    const auto& fc = config_.filtering_conf;

    if( has_preprocess )
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

void ActiveContourWorker::initializeFromInput(const QImage& input)
{
    timer_->stop();
    setState( WorkerState::Initializing );

    config_ = AppSettings::instance().imgSessSettings;
    inputImage_ = input;

    processImage();

    initializeActiveContour();

    if ( state_ != WorkerState::Ready )
        return;

    updateStats();
    emitContourOnly();

    initialShown_ = true;
}

void ActiveContourWorker::initializeActiveContour()
{
    if( processedImage_.isNull() )
        return;

    timer_->stop();
    setState( WorkerState::Initializing );

    ac_.reset();

    ofeli_ip::ContourData initialCD(initialPhi_.constBits(),
                                    initialPhi_.width(),
                                    initialPhi_.height(),
                                    config_.img_algo_conf.connectivity);

    bool is_rgb = ( processedImage_.format() != QImage::Format_Grayscale8 );

    const auto processedImg = image_span_from_qimage( processedImage_ );


    if( is_rgb )
    {
        ac_ = std::make_unique<ofeli_ip::RegionColorAc>(processedImg,
                                                        std::move(initialCD),
                                                        config_.img_algo_conf.ac_config,
                                                        config_.img_algo_conf.region_ac_config);
    }
    else
    {
        ac_ = std::make_unique<ofeli_ip::RegionAc>(processedImg,
                                                   std::move(initialCD),
                                                   config_.img_algo_conf.ac_config,
                                                   config_.img_algo_conf.region_ac_config);
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
    emitContourOnly();

    initializeActiveContour();
}

void ActiveContourWorker::suspend()
{
    if ( state_ == WorkerState::Running )
    {
        timer_->stop();
        setState( WorkerState::Suspended );

        updateStats();
        emitContourOnly();
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
        emitContourOnly();
    }
}

void ActiveContourWorker::updateStats()
{
    AlgoStats stats;
    stats.iteration = ac_ ? ac_->step_count() : 0;

    QMutexLocker lock(&statsMutex_);
    currentStats_ = stats;
}

void ActiveContourWorker::drawAndEmitResult()
{
    if( processedImage_.isNull() )
        return;

    QImage result = processedImage_.convertToFormat(QImage::Format_RGB32);

    const auto& display_config = config_.img_disp_conf;

    if ( ac_ == nullptr )
        return;

    const auto& l_out = ac_->l_out();
    const auto& l_in  = ac_->l_in();

    if ( display_config.l_out_displayed )
    {
        draw_list_to_img(l_out, display_config.l_out_color,
                         result);
    }

    if ( display_config.l_in_displayed )
    {
        draw_list_to_img(l_in, display_config.l_in_color,
                         result);
    }

    emit resultReady(result);
}

void ActiveContourWorker::emitContourOnly()
{
    // to test the former display
    //drawAndEmitResult();
        //return;

    if ( ac_ == nullptr )
        return;

    const auto& l_out = ac_->l_out();
    const auto& l_in  = ac_->l_in();

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

void ActiveContourWorker::reloadSettings()
{
    config_ = AppSettings::instance().imgSessSettings;

    processImage();
    initializeActiveContour();

    if ( state_ != WorkerState::Ready )
        return;

    updateStats();
    emitContourOnly();

    initialShown_ = true;
}

}
