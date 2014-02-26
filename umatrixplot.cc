#include "umatrixplot.h"
#include <QPen>
#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot_marker.h>
#include <qwt_color_map.h>
#include <qwt_text.h>
#include <qwt_scale_engine.h>
#include <qwt_raster_data.h>
#include <qwt_symbol.h>
#include <QSize>
#include <QMouseEvent>

#include <esom.h>
#include <vector>
#include <sstream>

UMatrixRasterData::UMatrixRasterData(esom::UMatrix& umatrix) 
  : QwtRasterData(), umatrix(umatrix) { 
  setBoundingRect(QwtDoubleRect(0.0, 0.0, umatrix.columns(), umatrix.rows()));
}

QwtDoubleInterval UMatrixRasterData::range() const {
  //  std::cerr << "min = " << min_v << " max = " << max_v << std::endl;
  return QwtDoubleInterval(min_v, max_v);
}

UMatrixLogRasterData::UMatrixLogRasterData(esom::UMatrix& umatrix)
  : UMatrixRasterData(umatrix) {
  double max = umatrix.getValue(0);
  double min = umatrix.getValue(0);
  for(unsigned int i=1; i < umatrix.size(); i++) {
    double v = umatrix.getValue(i);
    if(v == 0) continue;
    double lv = std::log(v);
    if(lv > max) 
      max_v = lv;
    if(lv < min)
      min_v = lv;
  }
}

QwtRasterData* UMatrixLogRasterData::copy() const {
  return new UMatrixLogRasterData(umatrix);
}

double UMatrixLogRasterData::value(double x, double y) const {
  unsigned int i = umatrix.coords2index(x, y);
  double v = umatrix.getValue(i);
  return (v == 0 ? min_v : std::log(v));
}

UMatrixLinearRasterData::UMatrixLinearRasterData(esom::UMatrix& umatrix)
  : UMatrixRasterData(umatrix) {
  double max = umatrix.getValue(0);
  double min = umatrix.getValue(0);
  for(unsigned int i=0; i < umatrix.size(); i++) {
    double v = umatrix.getValue(i);
    if(v > max)
      max_v = v;
    if(v < min)
      min_v = v;
  }
}

QwtRasterData* UMatrixLinearRasterData::copy() const {
  return new UMatrixLinearRasterData(umatrix);
}

double UMatrixLinearRasterData::value(double x, double y) const {
  unsigned int i = umatrix.coords2index(x, y);
  double v= umatrix.getValue(i);
  return v;
}

UMatrixPlot::UMatrixPlot(Dataset& dataset, esom::UMatrix& umatrix,
			 std::vector<int> bestmatches, QWidget *parent)
  : QwtPlot(parent), dataset(dataset), umatrix(umatrix),
    bestmatches(bestmatches) {
  
  linearData = new UMatrixLinearRasterData(umatrix);
  logData = new UMatrixLogRasterData(umatrix);
  rasterData = logData;

  spectro = new QwtPlotSpectrogram();

  //  QwtLinearColorMap colorMap(Qt::darkBlue, Qt::white);
  QwtLinearColorMap colorMap(Qt::blue, Qt::red);
  colorMap.addColorStop(0.5, Qt::white);
  // colorMap.addColorStop(1e-4, Qt::red);
  // colorMap.addColorStop(0.25, Qt::red);
  // colorMap.addColorStop(0.25, Qt::green);
  spectro->setColorMap(colorMap);

  //  rasterData.setBoundingRect(spectro->boundingRect());
  spectro->setData(*rasterData);
  spectro->attach(this);
  //  QBrush sbrush(QColor(255, 255, 255));
  //  QPen spen(QColor(255, 255, 255));
  QBrush sbrush(QColor(0, 0, 0));
  QPen spen(QColor(0, 0, 0));
  QSize ssize(5, 5);
  labelBoxes.resize(bestmatches.size());
  for(unsigned int i=0; i < bestmatches.size(); i++) {
    QwtPlotMarker *marker = new QwtPlotMarker();
    int bm = bestmatches[i];
    unsigned int x = umatrix.index2col(bm);
    unsigned int y = umatrix.index2row(bm);
    //    std::cerr << "y = " << y << std::endl;
    QwtSymbol symbol(QwtSymbol::Ellipse, sbrush, spen, ssize);
    std::stringstream label;
    label << dataset.row_labels.id(i); // dataset.names[i];
    QwtText label_text(label.str().c_str(), QwtText::PlainText);
    marker->setSymbol(symbol);
    marker->setValue(x, y);
    marker->setLabel(label_text);
    marker->setLabelAlignment(Qt::AlignRight);
    marker->attach(this);
    QSize size = label_text.textSize();
    //    std::cerr << "width: " << size.width() << " height: " << size.height() << std::endl;
    QRect box(x, y, size.width()+marker->spacing()+symbol.size().width(), size.height());
    labelBoxes[i] = box;
  }
}

void UMatrixPlot::mousePressEvent(QMouseEvent *event) {
  QPoint canvas_pos = canvas()->pos();
  int x = event->x() - canvas_pos.x();
  int y = event->y() - canvas_pos.y();
  //  std::cerr << "mouse " << x << ',' << y << std::endl;
  for(unsigned int i=0; i < labelBoxes.size(); i++) {
    QRect rBox = labelBoxes[i];
    QPoint tl = rBox.topLeft();
    //    std::cerr << "tl: " << tl.x() << ',' << tl.y();
    tl.setX(transform(QwtPlot::xBottom, tl.x()));
    tl.setY(transform(QwtPlot::yLeft, tl.y()) - rBox.height()/2);
    rBox.moveTopLeft(tl);
    //    std::cerr << " -> " << tl.x() << ',' << tl.y() << std::endl;
    //    std::cerr << "bl2: " << bl.x() << ',' << bl.y() << std::endl;
    if(rBox.contains(x, y)) {
      emit select(i);
      //      std::cerr << "label: " << dataset.names[i];
      return;
    }
  }
}
