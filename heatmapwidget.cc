#include "heatmapwidget.h"
#include "heatmapplot.h"

#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QVBoxLayout>
#include <QPrinter>
#include <QPrintDialog>

HeatmapWidget::HeatmapWidget(const MetabData& metab_data, const esom::LabelTree& tree, const MetabParams& params,
			     QWidget *parent)
  : QWidget(parent), metab_data(metab_data) {
  plot = new HeatmapPlot(metab_data, tree, params);
  QObject::connect(plot, SIGNAL(selectSample(int)), this, SIGNAL(selectSample(int)));
  QObject::connect(plot, SIGNAL(selectReplicate(int)), this, SIGNAL(selectReplicate(int)));
  QObject::connect(plot, SIGNAL(legend(QString)), this, SIGNAL(legend(QString)));
  QToolBar *toolbar = new QToolBar;
  QActionGroup *displayGroup = new QActionGroup(this);
  QAction *replicatesAct = new QAction("Replicates", displayGroup);
  replicatesAct->setCheckable(true);
  QAction *samplesAct = new QAction("Samples", displayGroup);
  samplesAct->setCheckable(true);
  QAction *sampleStdsAct = new QAction("Sample Stds", displayGroup);
  sampleStdsAct->setCheckable(true);

  if(!metab_data.samples.empty()) {
    samplesAct->setChecked(true);
    plot->showSamples();
  } else {
    replicatesAct->setChecked(true);
    samplesAct->setEnabled(false);
    sampleStdsAct->setEnabled(false);
    plot->showReplicates();
  }
  toolbar->addAction(replicatesAct);
  toolbar->addAction(samplesAct);
  toolbar->addAction(sampleStdsAct);
  QObject::connect(replicatesAct, SIGNAL(triggered()), plot, 
		   SLOT(showReplicates()));
  QObject::connect(samplesAct, SIGNAL(triggered()), plot, SLOT(showSamples()));
  QObject::connect(sampleStdsAct, SIGNAL(triggered()), plot, 
		   SLOT(showSampleStds()));

  QAction *printAct = new QAction("Print", toolbar);
  toolbar->addAction(printAct);
  QObject::connect(printAct, SIGNAL(triggered()), SLOT(print()));

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(toolbar);
  layout->addWidget(plot);
  this->setLayout(layout);
}

void HeatmapWidget::print() {
  QPrinter printer;
  if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
    plot->print(printer);
  }
}
