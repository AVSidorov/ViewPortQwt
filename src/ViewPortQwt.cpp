#include "ViewPortQwt.h"


#include <QwtRasterData>

namespace
{
}

struct ViewPortQwt::Data{
    Data(){};
};

ViewPortQwt::ViewPortQwt(QWidget *parent) : QwtPlot(parent), m(new Data()) {
}
