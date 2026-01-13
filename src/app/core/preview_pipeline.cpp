#include "preview_pipeline.hpp"

namespace ofeli_app {

PreviewPipeline::PreviewPipeline(QObject *parent)
    : QObject(parent)
{
}

void PreviewPipeline::setSourceImage(const QImage& img)
{
    m_background = img;
}


const QImage& PreviewPipeline::background() const
{
    return m_background;
}

}
