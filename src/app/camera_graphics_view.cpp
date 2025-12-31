#include "camera_graphics_view.hpp"
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPixmap>
#include <QFont>
#include <QTimer>
#include <QScrollBar>
#include <QWheelEvent>
#include <QLayout>

namespace ofeli_gui {

CameraGraphicsView::CameraGraphicsView(QWidget* parent)
    : QGraphicsView(parent),
      scene(new QGraphicsScene(this)),
      pixmapItem(nullptr),
      isPainting(false),
      isFullScreenMode(false)
{
    setScene(scene);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setAlignment(Qt::AlignCenter);

    setTransformationAnchor(QGraphicsView::NoAnchor);
    setResizeAnchor(QGraphicsView::NoAnchor);
    setDragMode(QGraphicsView::NoDrag);
}

void CameraGraphicsView::displayImage(const QImage& img)
{
    {
        QMutexLocker locker(&frameMutex);
        lastFrame = img;

        if (isPainting)
            return; // une autre frame est en train d'être affichée

        isPainting = true;
    }

    QTimer::singleShot(0, this, [this]() {
        QImage imgToDisplay;
        {
            QMutexLocker locker(&frameMutex);
            imgToDisplay = lastFrame;
        }

        // Dessine les stats sur l'image
        QImage imgWithStats = imgToDisplay;
        QPainter painter(&imgWithStats);

        drawTextOverlay(painter, imgToDisplay,
                        QString("FPS entrée: %1").arg(inputFps,0,'f',1), QPoint(10,30), 16);

        drawTextOverlay(painter, imgToDisplay,
                        QString("FPS traitement: %1").arg(processingFps,0,'f',1), QPoint(10,60), 16);

        drawTextOverlay(painter, imgToDisplay,
                        QString("FPS affichage: %1").arg(displayFps,0,'f',1), QPoint(10,90), 16);

        drawTextOverlay(painter, imgToDisplay,
                        QString("Drop rate: %1 %").arg(dropRate*100.f,0,'f',1), QPoint(10,120), 16);

        drawTextOverlay(painter, imgToDisplay,
                        QString("Latence moyenne: %1 ms").arg(avgLatencyMs,0,'f',1), QPoint(10,150), 16);

        drawTextOverlay(painter, imgToDisplay,
                        QString("Latence max: %1 ms").arg(maxLatencyMs,0,'f',1), QPoint(10,180), 16);

        painter.end();

        if (!pixmapItem) {
            pixmapItem = scene->addPixmap(QPixmap::fromImage(imgWithStats));
        } else {
            pixmapItem->setPixmap(QPixmap::fromImage(imgWithStats));
        }
        scene->setSceneRect(pixmapItem->boundingRect());

        QMutexLocker locker(&frameMutex);
        isPainting = false;
    });
}

// --- Fonction utilitaire pour couleur de texte selon fond
QColor CameraGraphicsView::contrastColorFast(const QImage &img, const QRect &area)
{
    int r=0,g=0,b=0, n=0;
    for(int y = area.top(); y < area.bottom() && y < img.height(); y+=4)
    {
        for(int x = area.left(); x < area.right() && x < img.width(); x+=4)
        {
            QRgb px = img.pixel(x,y);
            r += qRed(px); g += qGreen(px); b += qBlue(px);
            n++;
        }
    }
    if(n==0) return Qt::green;
    int luminance = (r+g+b)/3/n;
    return luminance > 128 ? Qt::black : Qt::green; // noir sur clair, vert sur sombre
}

void CameraGraphicsView::drawTextOverlay(QPainter &painter, const QImage &img,
                                         const QString &text, const QPoint &pos,
                                         int fontSize)
{
    // Couleur du texte principale
    QColor txtColor = contrastColorFast(img, QRect(pos.x(), pos.y()-fontSize, 150, fontSize+10));

    // Couleur du contour (opposée)
    QColor haloColor = (txtColor == Qt::black) ? Qt::white : Qt::black;

    QFont font("Arial", fontSize, QFont::Bold);
    painter.setFont(font);

    QPen haloPen(haloColor);
    haloPen.setWidth(2);
    painter.setPen(haloPen);

    // Dessin du contour autour du texte (offsets)
    const QPoint offsets[] = { {-1,-1}, {1,-1}, {-1,1}, {1,1} };
    for(const QPoint &off : offsets)
        painter.drawText(pos + off, text);

    // Dessin du texte principal
    painter.setPen(txtColor);
    painter.drawText(pos, text);
}

void CameraGraphicsView::setInputFps(float fps)
{
    QMutexLocker locker(&frameMutex);
    inputFps = fps;
}

void CameraGraphicsView::setProcessingFps(float fps)
{
    QMutexLocker locker(&frameMutex);
    processingFps = fps;
}

void CameraGraphicsView::setDisplayFps(float fps)
{
    QMutexLocker locker(&frameMutex);
    displayFps = fps;
}

void CameraGraphicsView::setDropRate(float dr)
{
    QMutexLocker locker(&frameMutex);
    dropRate = dr;
}

void CameraGraphicsView::setAvgLatencyMs(float lat)
{
    QMutexLocker locker(&frameMutex);
    avgLatencyMs = lat;
}

void CameraGraphicsView::setMaxLatencyMs(float lat)
{
    QMutexLocker locker(&frameMutex);
    maxLatencyMs = lat;
}

void CameraGraphicsView::wheelEvent(QWheelEvent* event)
{
    constexpr double zoomFactor = 1.15;

    // Position curseur avant zoom (en coordonnées scène)
    QPointF scenePosBeforeZoom = mapToScene(event->position().toPoint());

    // Sens de la molette
    double factor = (event->angleDelta().y() > 0)
                        ? zoomFactor
                        : 1.0 / zoomFactor;

    // Clamp du zoom
    double newZoom = currentZoom * factor;
    if (newZoom < minZoom || newZoom > maxZoom)
        return;

    currentZoom = newZoom;

    // Applique le zoom
    scale(factor, factor);

    // Position curseur après zoom
    QPointF scenePosAfterZoom = mapToScene(event->position().toPoint());

    // Décalage à compenser
    QPointF delta = scenePosAfterZoom - scenePosBeforeZoom;

    // Translate pour garder le point sous le curseur
    translate(delta.x(), delta.y());

    if (currentZoom > 1.01)
        setDragMode(QGraphicsView::ScrollHandDrag);
    else
        setDragMode(QGraphicsView::NoDrag);
}

void CameraGraphicsView::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    if (!isFullScreenMode) {
        // Sauvegarde état normal
        normalGeometry    = geometry();
        normalWindowFlags = windowFlags();

        // Passage plein écran
        setWindowFlags(Qt::Window);
        showFullScreen();

        isFullScreenMode = true;
    }
    else {
        // Retour état normal
        setWindowFlags(normalWindowFlags);
        showNormal();
        QTimer::singleShot(0, this, [this]() {
            setGeometry(normalGeometry);
        });

        isFullScreenMode = false;
    }
}

void CameraGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        resetTransform();
        currentZoom = 1.0;

        if (pixmapItem)
            fitInView(pixmapItem, Qt::KeepAspectRatio);

        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

} // namespace ofeli_gui
