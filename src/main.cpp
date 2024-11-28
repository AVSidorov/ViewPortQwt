
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QDebug>
#include <QRect>
#include <QGridLayout>
#include <QPainter>
#include <QPainterPath>
#include <QCursor>

#include <QwtScaleDiv>
#include <QwtMatrixRasterData>
#include <QwtPlotGrid>
#include <QwtInterval>
#include <QwtLinearColorMap>
#include <QwtScaleWidget>
#include <QwtPlotLayout>
#include <QwtPlotCanvas>
#include <QwtPlotZoomer>
#include <QwtPlotPanner>
#include <QwtPlotMagnifier>
#include <QwtPlotPicker>
#include <QwtScaleEngine>
#include <QwtPickerDragRectMachine>
#include <QwtPainter>
#include <QwtWidgetOverlay>
#include <QwtPlotSpectrogram>
#include <QwtRasterData>

#include "QwtRasterImage.h"

namespace
{
    class MainWindow : public QMainWindow
    {
    public:
        MainWindow(QWidget * = NULL);

    private:
        QVector<double> *data;

    };

    class Grid : public QwtPlotGrid
    {
    public:
        Grid()
        {
            setMajorPen(QPen(Qt::black));
            setMinorPen(QPen(Qt::gray));
        }
    };

    class ColorMap1 : public QwtLinearColorMap
    {
    public:
        ColorMap1(int n = 18) :
                QwtLinearColorMap(Qt::color0,QColor(static_cast<Qt::GlobalColor>(n)))
        {
            for (int i = 1; i <= n; i++)
            {
                addColorStop((double)i/(double)n, QColor(static_cast<Qt::GlobalColor>(i)));
            }
        }
    };

    class ColorMap : public QwtLinearColorMap
    {
    public:
        ColorMap() :
                QwtLinearColorMap(Qt::blue, Qt::red)
        {
            addColorStop(0.5, Qt::green);
        }
    };

    class ColorMapLog : public QwtLinearColorMap
    {
    public:
        ColorMapLog(int n = 10) :
                QwtLinearColorMap(Qt::blue, Qt::red)
        {
            auto jet = QwtLinearColorMap(Qt::blue, Qt::red);
            jet.addColorStop(0.5, Qt::green);

            // get colors
            QList<QRgb> linCol;
            for (auto i = 1; i < n; i++)
            {
                linCol << jet.rgb({0.,1.}, i/(double)n);
            }

            // shift color positions
            auto q = pow(1000., 1. / (double)n);
            for (auto i = 1; i < n; i++)
            {
                addColorStop(pow(q, (double)i) / 1000., linCol[i-1]);
            }
        }
    };

    class MyZoomer : public QwtPlotZoomer
    {
    public:
        MyZoomer(QwtPlotCanvas *canvas) :
                QwtPlotZoomer(canvas)
        {
            setTrackerMode(AlwaysOn);

        }

        virtual QwtText trackerTextF(const QPointF &pos) const
        {
            QColor bg(Qt::white);
            bg.setAlpha(200);

            QwtText text = QwtPlotZoomer::trackerTextF(pos);
            text.setBackgroundBrush(QBrush(bg));
            return text;
        }

        void drawRubberBand(QPainter *painter) const override
        {
            QBrush b{{10, 20, 255, 125}};
            painter->fillRect(painter->window(), b);
        };

        QRegion rubberBandMask() const override
        {
            return {QwtPicker::rubberBandMask().boundingRect()};
        }

    protected:
        QPolygon adjustedPoints(const QPolygon &polygon) const override
        {
            auto pos = canvas()->mapFromGlobal(QCursor::pos());
            auto width = canvas()->geometry().width();
            auto height = canvas()->geometry().height();
            if(polygon.size()<2)
            {
                return polygon;
            }
            if ((pos.x() < 0.05 * width || 0.95 * width < pos.x())
                && 0.05 * height < pos.y() && pos.y() < 0.95 * height) {
                QVector<QPoint> points;
                points.push_back(QPoint(0,polygon.point(0).y()));
                points.push_back(QPoint(width,polygon.point(1).y()));
                return {points};
            } else if (0.05 * width < pos.x() && pos.x() < 0.95 * width
                       && (pos.y() < 0.05 * height || 0.95 * height < pos.y())) {
                QVector<QPoint> points;
                points.push_back(QPoint(polygon.point(0).x(),0));
                points.push_back(QPoint(polygon.point(1).x(), height));
                return {points};
            } else {
                return polygon;
            }

        }
    };

    class MyPicker : public QwtPlotPicker
    {
    public:
        MyPicker(QwtPlotCanvas *canvas) : QwtPlotPicker(canvas)
        {
            setEnabled(true);
            setTrackerMode(AlwaysOn);
            setStateMachine(new QwtPickerDragRectMachine);
            setRubberBand(QwtPicker::RectRubberBand);
        }

        void drawRubberBand(QPainter *painter) const override
        {
            QBrush b{{222, 222, 222, 125}};
            painter->fillRect(painter->window(), b);
        };

        QRegion rubberBandMask() const override
        {
            return {QwtPicker::rubberBandMask().boundingRect()};
        }

    protected:
        QPolygon adjustedPoints(const QPolygon &polygon) const override
        {
            auto pos = canvas()->mapFromGlobal(QCursor::pos());
            auto width = canvas()->geometry().width();
            auto height = canvas()->geometry().height();
            if ((pos.x() < 0.05 * width || 0.95 * width < pos.x())
                && 0.05 * height < pos.y() && pos.y() < 0.95 * height) {
                QVector<QPoint> points;
                points.push_back(QPoint(0,polygon.point(0).y()));
                points.push_back(QPoint(width,polygon.point(1).y()));
                return {points};
            } else if (0.05 * width < pos.x() && pos.x() < 0.95 * width
                       && (pos.y() < 0.05 * height || 0.95 * height < pos.y())) {
                QVector<QPoint> points;
                points.push_back(QPoint(polygon.point(0).x(),0));
                points.push_back(QPoint(polygon.point(1).x(), height));
                return {points};
            } else {
                return polygon;
            }
        }
    };

    class TestData: public QwtRasterData
    {
    public:
        QwtInterval interval(Qt::Axis axis) const override
        {
            if(axis == Qt::ZAxis){
                return {value(1.,1.), value(0.,0.)};
            }
            return {-1,1};
        }

        double value(double x, double y) const override
        {
            return 999.9*exp(-x*x/0.03)*exp(-y*y/0.001) + 0.1;
        }
    };

    class TestDataLog: public QwtRasterData
    {
    public:
        QwtInterval interval(Qt::Axis axis) const override
        {
            if(axis == Qt::ZAxis){
                return {value(1.,1.), value(0.,2.)};
            }
            else if (axis == Qt::XAxis){
                return {-1.,1.};
            }
            else if (axis == Qt::YAxis){
                return {1.,3.};
            }
            return {-1,1};
        }

        double value(double x, double y) const override
        {
            auto minVal = 999.9*exp(-1/0.03)*exp(-1/0.001) + 0.1;
            return log((999.9*exp(-x*x/0.03)*exp(-(y-2)*(y-2)/0.001) + 0.1)/minVal);
        }
    };

}

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), data(new QVector<double>())
{
    auto plot = new QwtPlot(this);

    auto grid = new Grid();
    grid->setZ(12);
    grid->attach(plot);

    QImage image;
    image.load("lena.png");
    image.convertTo(QImage::Format_ARGB32);
    image.setDotsPerMeterX(image.width());
    image.setDotsPerMeterY(image.height());

    auto imageItem = new QwtRasterImage(image, {0.1 * image.width() / image.dotsPerMeterX(),
                                                0.5 * image.height() / image.dotsPerMeterY()});
    imageItem->setZ(1);
//    imageItem->attach(plot);

    auto image1 = QImage(image);
    image1.setDotsPerMeterX(5120);
    image1.setDotsPerMeterY(5120);
    auto imageItem1 = new QwtRasterImage(image1, {0., 0.});

    imageItem1->setZ(2);
    imageItem1->setAlpha(255);
//    imageItem1->attach(plot);

    auto gauss = new QwtPlotSpectrogram();
    gauss->setData(new TestData());
    gauss->attach(plot);
    // gauss->setZ(3);
    // gauss->setAlpha(125);
    gauss->setColorMap(new ColorMapLog());

    auto gaussLog = new QwtPlotSpectrogram();
    gaussLog->setData(new TestDataLog());
    gaussLog->attach(plot);
    // gauss->setZ(3);
    // gauss->setAlpha(125);
    gaussLog->setColorMap(new ColorMap());

    // auto  zInterval = QwtInterval{0, 255};
    // auto zInterval = gauss->interval(Qt::ZAxis);
    // auto scale = QwtLinearScaleEngine().divideScale(zInterval.minValue(), zInterval.maxValue(), 10, 5);

    // auto rightAxis = plot->axisWidget(QwtPlot::yRight);
    // rightAxis->setColorBarEnabled(true);
    // rightAxis->setColorMap(zInterval, new ColorMap());
    // rightAxis->setColorBarWidth(30);

    // plot->setAxisVisible(QwtPlot::yRight);
    // plot->setAxisAutoScale(QwtPlot::yRight, false);
    // plot->setAxisScaleDiv(QwtPlot::yRight, scale);
    // plot->setAxisTitle(QwtPlot::yRight, "z Scale");


     // auto zScaleWidget = new QwtScaleWidget(QwtScaleDraw::RightScale, this);
     // zScaleWidget->setTitle("Intensity");
     // zScaleWidget->setColorBarEnabled(true);
    // zScaleWidget->setColorMap(zInterval, new ColorMap());
    // zScaleWidget->setColorBarWidth(30);
    // zScaleWidget->setScaleDiv(scale);

    auto zInterval = QwtInterval{gauss->interval(Qt::ZAxis).minValue(), gauss->interval(Qt::ZAxis).maxValue()};
    auto scale = QwtLinearScaleEngine().divideScale(zInterval.minValue(), zInterval.maxValue(), 10, 5);
    auto scaleLog = QwtLogScaleEngine().divideScale(zInterval.minValue(), zInterval.maxValue(), 10, 10);

    auto zScaleWidgetLog = new QwtScaleWidget(QwtScaleDraw::RightScale, this);
    zScaleWidgetLog->setTitle("Intensity Log");
    zScaleWidgetLog->setColorBarEnabled(true);
    zScaleWidgetLog->setColorMap(zInterval, new ColorMapLog());
    zScaleWidgetLog->setColorBarWidth(15);
    zScaleWidgetLog->setScaleDiv(scale);


    auto zScaleWidget = new QwtScaleWidget(QwtScaleDraw::RightScale);
    zScaleWidget->setTitle("Intensity Linear");
    zScaleWidget->setColorBarEnabled(true);
    zScaleWidget->setColorMap(zInterval, new ColorMap());
    zScaleWidget->setColorBarWidth(15);
    zScaleWidget->setScaleDiv(scale);

    auto zScaleWidgetLogLog = new QwtScaleWidget(QwtScaleDraw::RightScale);
    zScaleWidgetLogLog->setTitle("Intensity Log Log");
    zScaleWidgetLogLog->setColorBarEnabled(true);
    zScaleWidgetLogLog->setColorMap(zInterval, new ColorMapLog());
    zScaleWidgetLogLog->setColorBarWidth(15);
    zScaleWidgetLogLog->setScaleDiv(scaleLog);

    auto zScaleWidgetLinLog = new QwtScaleWidget(QwtScaleDraw::RightScale);
    zScaleWidgetLinLog->setTitle("Intensity Linear Log");
    zScaleWidgetLinLog->setColorBarEnabled(true);
    zScaleWidgetLinLog->setColorMap(zInterval, new ColorMap());
    zScaleWidgetLinLog->setColorBarWidth(15);
    zScaleWidgetLinLog->setScaleDiv(scaleLog);


    // scale = QwtLogScaleEngine().divideScale(zIntervalLog.minValue()  , zIntervalLog.maxValue(), 10, 10);
    // zScaleWidgetLog->setScaleDiv(scale);

    auto *magnifier = new QwtPlotMagnifier(plot->canvas());
    magnifier->setAxisEnabled(Qt::YAxis, false);

    auto *panner = new QwtPlotPanner(plot->canvas());
    panner->setAxisEnabled(QwtPlot::yRight, false);
    panner->setMouseButton(Qt::LeftButton, Qt::ControlModifier);

    auto canvas = dynamic_cast<QwtPlotCanvas *>(plot->canvas());
    if (canvas) {
        auto *zoomer = new MyZoomer(canvas);
        auto *picker = new MyPicker(canvas);
        picker->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ShiftModifier);
    }

    // Avoid jumping when labels with more/less digits
    // appear/disappear when scrolling vertically

    const QFontMetrics fm(plot->axisWidget(QwtPlot::yLeft)->font());
    QwtScaleDraw *sd = plot->axisScaleDraw(QwtPlot::yLeft);
    sd->setMinimumExtent(fm.width("100.00"));


    auto gLayout = new QGridLayout();
    gLayout->addWidget(plot, 0, 0);
    gLayout->addWidget(zScaleWidgetLog, 0, 1);
    gLayout->addWidget(zScaleWidget, 0, 2);
    gLayout->addWidget(zScaleWidgetLogLog, 0, 3);
    gLayout->addWidget(zScaleWidgetLinLog, 0, 4);

    QWidget *window = new QWidget();
    window->setLayout(gLayout);
    setCentralWidget(window);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto main = MainWindow(nullptr);
    main.resize(600, 400);
    main.show();
    return app.exec();
}
