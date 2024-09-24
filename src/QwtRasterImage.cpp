//
// Created by sid on 9/24/24.
//
#include "QwtRasterImage.h"

QwtRasterImage::QwtRasterImage(QImage image, QPointF origin) : m_image(image), m_origin(origin) {};

QwtInterval QwtRasterImage::interval(Qt::Axis axis) const
{
    switch (axis) {
        case Qt::XAxis:
            return {0 - m_origin.x(), (float)m_image.width() / (float)m_image.dotsPerMeterX() - m_origin.x()};
        case Qt::YAxis:
            return {0 - m_origin.y(), (float)m_image.height() / (float)m_image.dotsPerMeterY() - m_origin.y()};
        case Qt::ZAxis:
            return {};
    }
};


QwtScaleMap QwtRasterImage::imageMap(Qt::Orientation orientation, const QwtScaleMap &map, const QRectF &area,
                                     const QSize &imageSize,
                                     double pixelSize) const
{
    double p1, p2, s1, s2;

    if (orientation == Qt::Horizontal) {
        p1 = 0;
        p2 = m_image.width();
        s1 = -m_origin.x();
        s2 = (double)m_image.width() / (double)m_image.dotsPerMeterX() - m_origin.x();
    } else {
        p1 = 0;
        p2 = m_image.height();
        s1 = -m_origin.y();
        s2 = (double)m_image.height() / (double)m_image.dotsPerMeterY() - m_origin.y();
    }

    if (pixelSize > 0.0 || p2 == 1.0) {
        double off = 0.5 * pixelSize;
        if (map.isInverting())
            off = -off;

        s1 += off;
        s2 += off;
    } else {
        p2--;
    }

    if (map.isInverting() && (s1 < s2))
        qSwap(s1, s2);

    QwtScaleMap newMap = map;
    newMap.setPaintInterval(p1, p2);
    newMap.setScaleInterval(s1, s2);

    return newMap;
}


QImage QwtRasterImage::renderImage(const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &area,
                                   const QSize &imageSize) const

{
    auto areaScaled = QwtScaleMap::transform(xMap, yMap, area);
    return m_image.copy(areaScaled.toRect()).scaled(imageSize);;
};

