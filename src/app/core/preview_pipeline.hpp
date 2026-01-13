#ifndef PREVIEW_PIPELINE_HPP
#define PREVIEW_PIPELINE_HPP

#include <QImage>
#include <QObject>

namespace ofeli_app {

class PreviewPipeline : public QObject {
    Q_OBJECT
public:
    PreviewPipeline(QObject* parent);
    void setSourceImage(const QImage& img);
    const QImage& background() const;

signals:
    void previewBackgroundChanged(const QImage& img);

private:
    QImage m_background;
};

}

#endif // PREVIEW_PIPELINE_HPP
