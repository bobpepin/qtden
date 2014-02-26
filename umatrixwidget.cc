#include "umatrixwidget.h"
#include "umatrixplot.h"

#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QVBoxLayout>
#include <QPrinter>
#include <QPrintDialog>

UMatrixWidget::UMatrixWidget(Dataset& dataset, esom::UMatrix& umatrix, 
			     std::vector<int> bestmatches,
			     QWidget *parent)
  : QWidget(parent) {
  plot = new UMatrixPlot(dataset, umatrix, bestmatches);

  QObject::connect(plot, SIGNAL(select(int)), this, SIGNAL(select(int)));
  //  QObject::connect(plot, SIGNAL(legend(QString)), this, SIGNAL(legend(QString)));
  QToolBar *toolbar = new QToolBar;

  QActionGroup *displayGroup = new QActionGroup(this);
  QAction *linearAct = new QAction("Linear", displayGroup);
  linearAct->setCheckable(true);
  QAction *logAct = new QAction("Logarithmic", displayGroup);
  logAct->setCheckable(true);
  logAct->setChecked(true);
  QObject::connect(linearAct, SIGNAL(triggered()), plot, SLOT(setLinearScale()));
  QObject::connect(logAct, SIGNAL(triggered()), plot, SLOT(setLogScale()));
  toolbar->addAction(linearAct);
  toolbar->addAction(logAct);

  QAction *printAct = new QAction("Print", toolbar);
  toolbar->addAction(printAct);
  QObject::connect(printAct, SIGNAL(triggered()), SLOT(print()));

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(toolbar);
  layout->addWidget(plot);
  this->setLayout(layout);
}

void UMatrixWidget::print() {
  QPrinter printer;
  if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
    plot->print(printer);
  }
}
