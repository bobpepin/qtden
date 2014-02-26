#include <cmath>
#include <sstream>

#include "vectorplot.h"
#include <QPen>
#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>
#include <qwt_scale_engine.h>
#include <QSize>
#include <QMouseEvent>

#include <esom.h>

#include "stat.h"

VectorPlot::VectorPlot(const MetabData& metab_data, 
		       QWidget *parent) 
  : QwtPlot(parent), metab_data(metab_data), dataset(&metab_data.samples), 
    errorbars(0, 0, 0), column_order(0), activeRow(-1) {
  setMouseTracking(true);
  canvas()->setMouseTracking(true);
}

void VectorPlot::displayRow(int row) {
  esom::Vector v = dataset->data(row);
  //  double center = mean(v);

  activeRow = row;

  column_order.resize(dataset->columns());
  for(unsigned int i=0; i < column_order.size(); i++) {
    column_order[i] = i;
  }

  clear();
  if(row == -1)
    return;
  QString title(dataset->row_labels.name(row).c_str());
  setTitle(title);
  setAxisScale(QwtPlot::xBottom, 0, dataset->dimension()-1, 0);
  //  std::cerr << "row = " << row << std::endl;
  QwtScaleEngine *se = axisScaleEngine(QwtPlot::yLeft);
  se->setReference(0);
  se->setAttribute(QwtScaleEngine::IncludeReference);
  //  double indices[v.size()];
  unsigned int groupCount = dataset->column_labels.countGroups();
  for(unsigned int k=0; k < column_order.size(); k++) {
    //    indices[k] = k;
    double index = k;
    unsigned int i = column_order[k];
    QwtPlotCurve *curve = new QwtPlotCurve();
    curve->setData(&index, (double*)v+i, 1);
    int group = dataset->column_labels.group(i);
    double h = (double)group / (double)groupCount;
    //    std::cerr << "group = " << group << std::endl;
    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidth(10);
    pen.setColor(QColor::fromHsvF(h, 1.0, 1.0));
    curve->setPen(pen);
    curve->setStyle(QwtPlotCurve::Sticks);
    curve->attach(this);
    if(!errorbars.rows()) continue;
    QwtPlotMarker *ebar = new QwtPlotMarker;
    QwtSymbol sym;
    sym.setStyle(QwtSymbol::VLine);
    sym.setSize(QSize(10, 10));
    ebar->setSymbol(sym);
    ebar->setXValue(index);
    ebar->setYValue(errorbars(row, i));
  }
  replot();
}

void VectorPlot::mouseMoveEvent(QMouseEvent *event) {
  if(activeRow == -1) {
    event->ignore();
    return;
  }
  QPoint canvas_pos = canvas()->pos();
  QPoint pos = event->pos();
  double v = invTransform(QwtPlot::xBottom, pos.x() - canvas_pos.x());
  unsigned int i = std::floor(v + 0.5);
  if(i >= column_order.size()) {
    event->ignore();
    return;
  }
  std::stringstream label;
  int col = column_order[i];
  label << dataset->column_labels.name(col);
  label << ": " << dataset->data(activeRow, col);
  emit legend(QString(label.str().c_str()));
  //  std::cerr << " i=" << i << " label = " << label << std::endl;
}

QSize VectorPlot::sizeHint() const {
  return QSize(600, 300);
}

void VectorPlot::showReplicates() {
  dataset = &metab_data.replicates;
  displayRow(activeRow);
}

void VectorPlot::showSamples() {
  dataset = &metab_data.samples;
  displayRow(activeRow);
}
