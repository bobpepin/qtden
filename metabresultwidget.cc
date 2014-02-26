#include "metabresultwidget.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <cmath>

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <qwt_panner.h>

#include <esom.h>

#include "stat.h"
#include "metabdata.h"

#include "dendrogram.h"
#include "vectorplot.h"
#include "vectorplotwidget.h"
#include "umatrixplot.h"
#include "umatrixwidget.h"
#include "heatmapplot.h"
#include "heatmapwidget.h"
#include "param_window.h"
#include "resulttablewidget.h"

MetabResultWidget::MetabResultWidget(MetabAnalysis *result,
				     QWidget *parent) 
  : QMainWindow(parent), result(result) {
  setAttribute(Qt::WA_DeleteOnClose, true);

  Dataset& subset = result->metab_data.replicates;

  VectorPlotWidget *vp = new VectorPlotWidget(result->metab_data);

  Dendrogram *den = new Dendrogram(result->tree, subset);
  if(result->params.esom_data == MetabParams::Replicates) {
    QObject::connect(den, SIGNAL(select(int)), vp, SLOT(displayReplicateRow(int)));
  } else {
    QObject::connect(den, SIGNAL(select(int)), vp, SLOT(displaySampleRow(int)));
  }

  UMatrixWidget *ump = new UMatrixWidget(subset, *result->umatrix, result->bestmatches);
  if(result->params.esom_data == MetabParams::Replicates) {
    QObject::connect(ump, SIGNAL(select(int)), vp, SLOT(displayReplicateRow(int)));
  } else {
    QObject::connect(ump, SIGNAL(select(int)), vp, SLOT(displaySampleRow(int)));
  }
  QObject::connect(ump, SIGNAL(select(int)), den, SLOT(activateItem(int)));

  HeatmapWidget *hmw = new HeatmapWidget(result->metab_data, result->tree, result->params);
  QObject::connect(hmw, SIGNAL(selectReplicate(int)), vp, SLOT(displayReplicateRow(int)));
  QObject::connect(hmw, SIGNAL(selectSample(int)), vp, SLOT(displaySampleRow(int)));
  if(result->params.esom_data == MetabParams::Replicates) {
    QObject::connect(hmw, SIGNAL(selectReplicate(int)), den, SLOT(activateItem(int)));
  } else {
    QObject::connect(hmw, SIGNAL(selectSample(int)), den, SLOT(activateItem(int)));
  }

  ResultTableWidget *rtw = new ResultTableWidget(result->metab_data, 
						 result->tree);
  QObject::connect(rtw, SIGNAL(selectSample(int)), vp, SLOT(displaySampleRow(int)));
  QObject::connect(rtw, SIGNAL(selectReplicate(int)), vp, SLOT(displayReplicateRow(int)));
  if(result->params.esom_data == MetabParams::Replicates) {
    QObject::connect(rtw, SIGNAL(selectReplicate(int)), den, SLOT(activateItem(int)));
  } else {
    QObject::connect(rtw, SIGNAL(selectSample(int)), den, SLOT(activateItem(int)));
  }

  QTabWidget *tabs = new QTabWidget();
  tabs->addTab(ump, "UMatrix");
  tabs->addTab(rtw, "Features");
  tabs->addTab(den, "Dendrogram");
  tabs->addTab(hmw, "Heatmap");

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(tabs);
  layout->addWidget(vp);

  QWidget *window = new QWidget;
  window->setLayout(layout);

  QMainWindow *mainwindow = this;
  QToolBar *toolbar = mainwindow->addToolBar("Toolbar");
  QAction *printAct = new QAction("Print", mainwindow);
  toolbar->addAction(printAct);
  QObject::connect(printAct, SIGNAL(triggered()), den, SLOT(print()));

  QStatusBar *statusbar = mainwindow->statusBar();
  QObject::connect(vp, SIGNAL(legend(QString)), statusbar, SLOT(showMessage(QString)));
  QObject::connect(hmw, SIGNAL(legend(QString)), statusbar, SLOT(showMessage(QString)));

  //  mainwindow->setWindowTitle(argv[1]);
  mainwindow->setCentralWidget(window);
  mainwindow->addToolBar(Qt::TopToolBarArea, toolbar);

}

MetabResultWidget::~MetabResultWidget() {
  delete result;
}
