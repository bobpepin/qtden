#ifndef QTDEN_METAB_RESULTWIDGET
#define QTDEN_METAB_RESULTWIDGET

#include <vector>

#include <QMainWindow>

#include "param_window.h"
#include "metabdata.h"
#include "metabanalysis.h"

class MetabResultWidget : public QMainWindow {
  Q_OBJECT

 private:
  MetabAnalysis *result;
 public:
  MetabResultWidget(MetabAnalysis *result,
		    QWidget *parent = 0);
  ~MetabResultWidget();
};

#endif
