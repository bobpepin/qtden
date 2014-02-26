#include "vectorplotwidget.h"
#include "vectorplot.h"

#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QVBoxLayout>
#include <QPrinter>
#include <QPrintDialog>

VectorPlotWidget::VectorPlotWidget(const MetabData& metab_data, QWidget *parent)
  : QWidget(parent), metab_data(metab_data) {
  this->plot = new VectorPlot(metab_data);
  QObject::connect(plot, SIGNAL(legend(QString)), this, SIGNAL(legend(QString)));
  QToolBar *toolbar = new QToolBar;
  QActionGroup *displayGroup = new QActionGroup(this);
  replicatesAct = new QAction("Replicates", displayGroup);
  replicatesAct->setCheckable(true);
  samplesAct = new QAction("Samples", displayGroup);
  samplesAct->setCheckable(true);
  if(!metab_data.samples.empty()) {
    samplesAct->setChecked(true);
    plot->showSamples();
  } else {
    samplesAct->setEnabled(false);
    replicatesAct->setChecked(true);
    plot->showReplicates();
  }
  toolbar->addAction(replicatesAct);
  toolbar->addAction(samplesAct);
  QObject::connect(replicatesAct, SIGNAL(triggered()), plot, 
		   SLOT(showReplicates()));
  QObject::connect(samplesAct, SIGNAL(triggered()), plot, SLOT(showSamples()));

  QAction *printAct = new QAction("Print", toolbar);
  toolbar->addAction(printAct);
  QObject::connect(printAct, SIGNAL(triggered()), SLOT(print()));

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(toolbar);
  layout->addWidget(plot);
  this->setLayout(layout);
}

void VectorPlotWidget::displaySampleRow(int r) {
  if(metab_data.samples_dimension != MetabData::Columns) {
    replicatesAct->setEnabled(false);
    samplesAct->setEnabled(true);
    samplesAct->setChecked(true);
    plot->displayRow(-1);
    plot->showSamples();
  }
  plot->displayRow(r);
}

void VectorPlotWidget::displayReplicateRow(int r) {
  if(metab_data.samples_dimension != MetabData::Columns) {
    samplesAct->setEnabled(false);
    replicatesAct->setEnabled(true);
    replicatesAct->setChecked(true);
    plot->displayRow(-1);
    plot->showReplicates();
  }
  plot->displayRow(r);
}

void VectorPlotWidget::print() {
  QPrinter printer;
  if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
    plot->print(printer);
  }
}

void VectorPlotWidget::displayRow(int r) {
  plot->displayRow(r);
}
