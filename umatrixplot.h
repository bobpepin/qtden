#ifndef QTDEN_UMATRIXPLOT_H
#define QTDEN_UMATRIXPLOT_H

#include <vector>

#include <QObject>
#include <qwt_double_interval.h>
#include <qwt_double_rect.h>
#include <qwt_raster_data.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_plot.h>
#include <esom.h>

#include "dataset.h"

class UMatrixRasterData : public QwtRasterData {
 protected:
  esom::UMatrix& umatrix;
  double min_v;
  double max_v;
 public:
  UMatrixRasterData(esom::UMatrix&);
  UMatrixRasterData(esom::UMatrix&, const QwtDoubleRect&);
  virtual QwtRasterData* copy() const = 0;
  QwtDoubleInterval range() const;
  virtual double value(double x, double y) const = 0;
};

class UMatrixLinearRasterData : public UMatrixRasterData {
 public:
  UMatrixLinearRasterData(esom::UMatrix&);
  QwtRasterData* copy() const;
  double value(double x, double y) const;
};

class UMatrixLogRasterData : public UMatrixRasterData {
 public:
  UMatrixLogRasterData(esom::UMatrix&);
  QwtRasterData* copy() const;
  double value(double x, double y) const;
};

class UMatrixPlot : public QwtPlot {
  Q_OBJECT
 private:
  Dataset& dataset;
  esom::UMatrix& umatrix;
  std::vector<int> bestmatches;
  UMatrixRasterData *rasterData;
  UMatrixLinearRasterData *linearData;
  UMatrixLogRasterData *logData;
  std::vector<QRect> labelBoxes;
  QwtPlotSpectrogram *spectro;
 public:
  UMatrixPlot(Dataset&, esom::UMatrix&, std::vector<int>, 
	      QWidget *parent=0);
 protected:
  void mousePressEvent(QMouseEvent*);

 public slots:
  void setLogScale() {
    rasterData = logData;
    spectro->setData(*rasterData);
    replot();
  }
  void setLinearScale() {
    rasterData = linearData;
    spectro->setData(*rasterData);
    replot();
  }


 signals:
  void select(int);
};

#endif
