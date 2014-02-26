#ifndef QTDEN_VECTOR_PLOT
#define QTDEN_VECTOR_PLOT

#include <vector>

#include <QObject>
#include <qwt_plot.h>
#include <esom.h>

#include "metabdata.h"

class VectorPlot : public QwtPlot {
  Q_OBJECT
 private:
  const MetabData& metab_data;
  const Dataset* dataset;
  esom::Matrix errorbars;
  std::vector<unsigned int> column_order;
  int activeRow;
 public:
  VectorPlot(const MetabData&, QWidget *parent=0);
  QSize sizeHint() const;
 signals:
  void legend(QString);
 public slots:
  void displayRow(int);
  void showReplicates();
  void showSamples();
 protected:
  void mouseMoveEvent(QMouseEvent*);
};

#endif
