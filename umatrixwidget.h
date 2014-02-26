#ifndef QTDEN_UMATRIX_WIDGET_H
#define QTDEN_UMATRIX_WIDGET_H

#include <QWidget>
#include <esom.h>

#include "metabdata.h"
#include "umatrixplot.h"

class UMatrixWidget : public QWidget {
  Q_OBJECT
 private:
  UMatrixPlot *plot;
 public:
  UMatrixWidget(Dataset&, esom::UMatrix&, std::vector<int>,
		QWidget *parent=0);
 public slots:
  void print();
 signals:
  void select(int);
  void legend(QString);
};

#endif
