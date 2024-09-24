/*****************************************************************************
 * Qwt Polar Examples - Copyright (C) 2008   Uwe Rathmann
 * This file may be used under the terms of the 3-clause BSD License
 *****************************************************************************/

#pragma once

#include <QwtPlot>


class ViewPortQwt: QwtPlot
{
    Q_OBJECT

  public:
    ViewPortQwt( QWidget* parent = NULL);
//  public slots:

  private:
    struct Data;
    Data* m;
};
