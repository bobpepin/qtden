#ifndef QTDEN_VECTORPLOT_WIDGET_H
#define QTDEN_VECTORPLOT_WIDGET_H

#include <QWidget>
#include <esom.h>

#include "metabdata.h"
#include "vectorplot.h"

class VectorPlotWidget : public QWidget {
  Q_OBJECT
 private:
  const MetabData& metab_data;
  VectorPlot *plot;
  QAction *replicatesAct;
  QAction *samplesAct;
 public:
  VectorPlotWidget(const MetabData& metab_data, QWidget *parent=0);
 signals:
  void legend(QString);
 public slots:
  void displayRow(int);
  void displaySampleRow(int);
  void displayReplicateRow(int);
  void print();
};

#endif
