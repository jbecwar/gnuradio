/* -*- c++ -*- */
/*
 * Copyright 2012,2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <cmath>
#include <QColorDialog>
#include <QMessageBox>
#include <timerasterdisplayform.h>
#include <iostream>

TimeRasterDisplayForm::TimeRasterDisplayForm(int nplots,
					     double samp_rate,
					     double rows, double cols,
					     double zmax,
					     QWidget* parent)
  : DisplayForm(nplots, parent)
{
  _layout = new QGridLayout(this);
  _displayPlot = new TimeRasterDisplayPlot(nplots, samp_rate, rows, cols, this);
  _layout->addWidget(_displayPlot, 0, 0);
  setLayout(_layout);

  _min_val =  10;
  _max_val = -10;

  // We don't use the normal menus that are part of the displayform.
  // Clear them out to get rid of their resources.
  for(int i = 0; i < nplots; i++) {
    _lines_menu[i]->clear();
  }
  _line_title_act.clear();
  _line_color_menu.clear();
  _line_width_menu.clear();
  _line_style_menu.clear();
  _line_marker_menu.clear();
  _marker_alpha_menu.clear();

  // Now create our own menus
  for(int i = 0; i < nplots; i++) {
    ColorMapMenu *colormap = new ColorMapMenu(i, this);
    connect(colormap, SIGNAL(whichTrigger(int, const int, const QColor&, const QColor&)),
	    this, SLOT(setColorMap(int, const int, const QColor&, const QColor&)));
    _lines_menu[i]->addMenu(colormap);

    _marker_alpha_menu.push_back(new MarkerAlphaMenu(i, this));
    connect(_marker_alpha_menu[i], SIGNAL(whichTrigger(int, int)),
	    this, SLOT(setAlpha(int, int)));
    _lines_menu[i]->addMenu(_marker_alpha_menu[i]);
  }

  QAction *autoscale_act = new QAction("Auto Scale", this);
  autoscale_act->setStatusTip(tr("Autoscale intensity range"));
  connect(autoscale_act, SIGNAL(triggered()), this, SLOT(autoScale()));
  _menu->addAction(autoscale_act);

  PopupMenu *colsmenu = new PopupMenu("Num. Columns", this);
  _menu->addAction(colsmenu);
  connect(colsmenu, SIGNAL(whichTrigger(QString)),
	  this, SLOT(setNumCols(QString)));

  PopupMenu *rowsmenu = new PopupMenu("Num. Rows", this);
  _menu->addAction(rowsmenu);
  connect(rowsmenu, SIGNAL(whichTrigger(QString)),
	  this, SLOT(setNumRows(QString)));

  getPlot()->setIntensityRange(0, zmax);

  Reset();

  connect(_displayPlot, SIGNAL(plotPointSelected(const QPointF)),
	  this, SLOT(onPlotPointSelected(const QPointF)));
}

TimeRasterDisplayForm::~TimeRasterDisplayForm()
{
  // Don't worry about deleting Display Plots - they are deleted when
  // parents are deleted
}

TimeRasterDisplayPlot*
TimeRasterDisplayForm::getPlot()
{
  return ((TimeRasterDisplayPlot*)_displayPlot);
}

double
TimeRasterDisplayForm::numRows()
{
  return getPlot()->numRows();
}

double
TimeRasterDisplayForm::numCols()
{
  return getPlot()->numCols();
}

void
TimeRasterDisplayForm::newData(const QEvent *updateEvent)
{
  TimeRasterUpdateEvent *event = (TimeRasterUpdateEvent*)updateEvent;
  const std::vector<double*> dataPoints = event->getPoints();
  const uint64_t numDataPoints = event->getNumDataPoints();

  _min_val =  10;
  _max_val = -10;
  for(size_t i=0; i < dataPoints.size(); i++) {
    double *min_val = std::min_element(&dataPoints[i][0], &dataPoints[i][numDataPoints-1]);
    double *max_val = std::max_element(&dataPoints[i][0], &dataPoints[i][numDataPoints-1]);
    if(*min_val < _min_val)
      _min_val = *min_val;
    if(*max_val > _max_val)
      _max_val = *max_val;
  }

  getPlot()->plotNewData(dataPoints, numDataPoints);
}

void
TimeRasterDisplayForm::customEvent( QEvent * e)
{
  if(e->type() == TimeRasterUpdateEvent::Type()) {
    newData(e);
  }
}

void
TimeRasterDisplayForm::setNumRows(double rows)
{
  getPlot()->setNumRows(rows);
}

void
TimeRasterDisplayForm::setNumRows(QString rows)
{
  getPlot()->setNumRows(rows.toDouble());
}

void
TimeRasterDisplayForm::setNumCols(double cols)
{
  getPlot()->setNumCols(cols);
}

void
TimeRasterDisplayForm::setNumCols(QString cols)
{
  getPlot()->setNumCols(cols.toDouble());
}

void
TimeRasterDisplayForm::setColorMap(int which,
				   const int newType,
				   const QColor lowColor,
				   const QColor highColor)
{
  getPlot()->setIntensityColorMapType(which, newType,
				      lowColor, highColor);
}

void
TimeRasterDisplayForm::setAlpha(int which, int alpha)
{
  getPlot()->setAlpha(which, alpha);
}

void
TimeRasterDisplayForm::setIntensityRange(const double minIntensity,
					const double maxIntensity)
{
  getPlot()->setIntensityRange(minIntensity, maxIntensity);
}

void
TimeRasterDisplayForm::autoScale()
{
  double min_int = _min_val;
  double max_int = _max_val;

  getPlot()->setIntensityRange(min_int, max_int);
}