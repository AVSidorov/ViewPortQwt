//
// Created by sid on 9/24/24.
//

#ifndef VIEWPORTQWT_QWTRASTERIMAGE_H
#define VIEWPORTQWT_QWTRASTERIMAGE_H

#include<QPointF>
#include<QImage>
#include<QwtInterval>
#include<QwtPlotRasterItem>
#include <QwtScaleMap>

class QwtRasterImage : public QwtPlotRasterItem
{
public:
    QwtRasterImage(QImage image, QPointF origin = {0., 0.});

protected:
    QwtScaleMap
    imageMap(Qt::Orientation orientation, const QwtScaleMap &map, const QRectF &area, const QSize &imageSize,
             double pixelSize) const override;
    QwtInterval interval(Qt::Axis axis) const override;

    QImage renderImage(const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &area,
                       const QSize &imageSize) const override;

private:
    QImage m_image;
    QPointF m_origin;
};


#endif //VIEWPORTQWT_QWTRASTERIMAGE_H
