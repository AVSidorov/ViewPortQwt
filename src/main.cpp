
#include <QApplication>
#include <QMainWindow>
#include <QToolBar>
#include <QToolButton>
#include <QwtPlotSpectrogram>
#include "ViewPortQwt.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <QDebug>
#include <QRect>
#include <QPainter>
#include <QPainterPath>


#include <QwtScaleDiv>
#include <QwtScaleMap>
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
#include <QwtScaleEngine>
#include <QGridLayout>
#include <QwtPlotRasterItem>
#include <QwtPlotZoomer>
#include <QwtPickerDragRectMachine>
#include <QwtPainter>
#include <QwtWidgetOverlay>

#include "QwtRasterImage.h"

namespace
{
    class MainWindow : public QMainWindow
    {
    public:
        MainWindow( QWidget* = NULL );
    private:
        QVector<double>* data;

    };

    class Grid: public QwtPlotGrid{
    public:
        Grid(){
            setMajorPen(QPen(Qt::black));
            setMinorPen(QPen(Qt::gray));
        }
    };
    class ColorMap: public QwtLinearColorMap
    {
    public:
        ColorMap():
                QwtLinearColorMap( Qt::blue, Qt::red )
        {
              addColorStop( 0.5, Qt::green );
        }
    };
    class MyZoomer: public QwtPlotZoomer {
    public:
        MyZoomer( QwtPlotCanvas *canvas ):
                QwtPlotZoomer( canvas )
        {
            setTrackerMode( AlwaysOn );

        }

        virtual QwtText trackerTextF( const QPointF &pos ) const
        {
            QColor bg( Qt::white );
            bg.setAlpha( 200 );

            QwtText text = QwtPlotZoomer::trackerTextF( pos );
            text.setBackgroundBrush( QBrush( bg ) );
            return text;
        }
        void drawRubberBand(QPainter* painter) const override {
            QBrush b {{10,20, 255, 125}};
            painter->fillRect(painter->window(),b);
        };

        QRegion rubberBandMask() const override {
            return {QwtPicker::rubberBandMask().boundingRect()};
        };

    };
    class MyPicker: public QwtPicker {
    public:
        MyPicker(QwtPlotCanvas *canvas ): QwtPicker(canvas) {
            setEnabled(true);
            setTrackerMode(AlwaysOn);
            setStateMachine(new QwtPickerDragRectMachine);
            setRubberBand(QwtPicker::RectRubberBand);
        }
         void drawRubberBand(QPainter* painter) const override {
            QBrush b {{10,20, 255, 125}};
            painter->fillRect(painter->window(),b);
        };

        QRegion rubberBandMask() const override {
            return {QwtPicker::rubberBandMask().boundingRect()};
        };
    };
}

MainWindow::MainWindow( QWidget* parent)
    : QMainWindow( parent ), data(new QVector<double>())
{
    auto plot = new QwtPlot(this);

    QImage image;
    image.load("/home/sid/prog/ViewPortQwt/lena.png");
    image.convertTo(QImage::Format_ARGB32);
    image.setDotsPerMeterX(image.width());
    image.setDotsPerMeterY(image.height());

    auto imageItem = new QwtRasterImage(image, {0.1*image.width()/image.dotsPerMeterX(),
                                                0.5*image.height()/image.dotsPerMeterY()});
    imageItem->setZ(1);
    imageItem->attach(plot);

    auto image1 = QImage(image);
    image1.setDotsPerMeterX(5120);
    image1.setDotsPerMeterY(5120);
    auto imageItem1 = new QwtRasterImage(image1, {0.,0.});

    imageItem1->setZ(2);
    imageItem1->setAlpha(255);
    imageItem1->attach(plot);

    plot->axisScaleEngine(QwtPlot::yLeft)->setAttribute(QwtScaleEngine::Inverted);

    auto grid = new Grid();
    grid->setZ(12);
    grid->attach(plot);

    const QwtInterval zInterval {0,255};
    auto rightAxis = new QwtScaleWidget(QwtScaleDraw::RightScale, this);
    rightAxis->setTitle( "Intensity" );
    rightAxis->setColorBarEnabled( true );
    rightAxis->setColorMap( zInterval, new ColorMap() );
    rightAxis->setColorBarWidth(30);
//    auto ticks = new QList<double>();
//    ticks->push_back( zInterval.minValue());
//    ticks->push_back((zInterval.minValue()-zInterval.minValue())/2);
//    ticks->push_back(zInterval.maxValue());
    auto scale = QwtLinearScaleEngine().divideScale(zInterval.minValue(), zInterval.maxValue(), 10, 5);
    rightAxis->setScaleDiv(scale);

    auto rightAxis1 = new QwtScaleWidget(QwtScaleDraw::RightScale, this);
    rightAxis1->setTitle( "Intensity1" );
    rightAxis1->setColorBarEnabled( true );
    rightAxis1->setColorMap( zInterval, new ColorMap() );
    rightAxis1->setColorBarWidth(15);

    scale = QwtLogScaleEngine().divideScale(1, 1000, 10, 5);
    rightAxis1->setScaleDiv(scale);

    auto* magnifier = new QwtPlotMagnifier(plot->canvas());
    magnifier->setMouseButton( Qt::RightButton, Qt::ShiftModifier );

    auto* panner = new QwtPlotPanner( plot->canvas());
    panner->setAxisEnabled( QwtPlot::yRight, false );
    panner->setMouseButton(Qt::MiddleButton);

    auto canvas = dynamic_cast<QwtPlotCanvas*>(plot->canvas());
    if(canvas) {
        auto* zoomer = new MyZoomer(canvas);
        auto* picker = new MyPicker(canvas);
    }

    // Avoid jumping when labels with more/less digits
    // appear/disappear when scrolling vertically

    const QFontMetrics fm( plot->axisWidget( QwtPlot::yLeft )->font() );
    QwtScaleDraw *sd = plot->axisScaleDraw( QwtPlot::yLeft );
    sd->setMinimumExtent( fm.width( "100.00" ) );


    auto gLayout = new QGridLayout();
    gLayout->addWidget(plot,0,0);
    gLayout->addWidget(rightAxis, 0, 1);
    gLayout->addWidget(rightAxis1, 0, 2);

    QWidget *window = new QWidget();
    window->setLayout(gLayout);
    setCentralWidget(window);
}

int main( int argc, char* argv[] )
{
    QApplication app( argc, argv );

    auto main = MainWindow(nullptr);
    main.resize( 600, 400 );
    main.show();
    return app.exec();
}
