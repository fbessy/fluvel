// SPDX-License-Identifier: CeCILL-2.1
// Copyright (C) 2010-2026 Fabien Bessy

#include <QImage>
#include <QMouseEvent>
#include <QPixmap>
#include <QRectF>
#include <QWidget>

namespace fluvel
{

class MiniMapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MiniMapWidget(QWidget* parent = nullptr);

    void setImage(const QImage& image);

    void setSceneRect(const QRectF& sceneRect);
    void setVisibleRect(const QRectF& visibleRect);

signals:
    void centerRequested(const QPointF& scenePosition);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    QPointF scenePositionFromMiniMap(const QPointF& position) const;

    QPixmap thumbnail_;

    QRectF sceneRect_;
    QRectF visibleRect_;
};

} // namespace fluvel