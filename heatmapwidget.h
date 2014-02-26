#ifndef QTDEN_HEATMAP_WIDGET_H
#define QTDEN_HEATMAP_WIDGET_H

#include <QWidget>
#include <esom.h>

#include "metabdata.h"
#include "heatmapplot.h"

class HeatmapWidget : public QWidget {
  Q_OBJECT
 private:
  const MetabData& metab_data;
  HeatmapPlot *plot;
 public:
  HeatmapWidget(const MetabData& metab_data, const esom::LabelTree&, const MetabParams&,
		QWidget *parent=0);
 public slots:
  void print();
 signals:
  void selectReplicate(int);
  void selectSample(int);
  void legend(QString);
};

#endif
