#ifndef QTDEN_HEATMAP_PLOT_H
#define QTDEN_HEATMAP_PLOT_H

#include <vector>

#include <QObject>
#include <qwt_double_interval.h>
#include <qwt_double_rect.h>
#include <qwt_raster_data.h>
#include <qwt_plot.h>
#include <esom.h>

#include "dataset.h"
#include "metabdata.h"
#include "metabanalysis.h"

class HeatmapRasterData : public QwtRasterData {
  const Dataset* dataset;
  const esom::LabelTree& tree;
 public:
  bool orderEnabled;
  std::vector<unsigned int> order;
  HeatmapRasterData(const Dataset*, const esom::LabelTree&);
  void setDataset(const Dataset*);
  void toggleOrder(bool);
  QwtRasterData* copy() const;
  QwtDoubleInterval range() const;
  double value(double x, double y) const;
};

class HeatmapPlot : public QwtPlot {
  Q_OBJECT
 private:
  const MetabData& metab_data;
  const Dataset* dataset;
  const Dataset* dataset_tstat;
  const esom::LabelTree& tree;
  const MetabParams& params;
  HeatmapRasterData rasterData;
  void mouse2data(QMouseEvent*, unsigned int&, unsigned int&);
  void plotDataset();
  bool order_enabled;
 public:
  HeatmapPlot(const MetabData&, const esom::LabelTree&, const MetabParams&, QWidget *parent=0);
 protected:
  void mousePressEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
 public slots:
  void showReplicates();
  void showSamples();
  void showSampleStds();
 signals:
  void selectSample(int);
  void selectReplicate(int);
  void legend(QString);
};

#endif
