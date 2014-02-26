#include <iostream>
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

#include <fdistr.h>

#include <esom.h>

#include "stat.h"
#include "meandataset.h"

#include "dendrogram.h"
#include "vectorplot.h"
#include "umatrixplot.h"
#include "heatmapplot.h"

void readLrn(const char* fname, Dataset& dataset);
void skipline(std::istream&, char term='\n');
std::vector<unsigned int> read_column_order(const char*);
void writeUMatrix(const char*, esom::UMatrix&);

std::ostream& operator<<(std::ostream& os, esom::Vector v) {
  for(unsigned int i=0; i < v.size(); i++) {
    os << v[i] << '\t';
  }
  return os;
}

std::vector<unsigned int> readHits(const char* prefix, Dataset& dataset) {
  std::string hitsfile = std::string(prefix) + ".hits";
  std::ifstream ifs(hitsfile.c_str());
  char c;
  unsigned int n;
  ifs >> c >> n;
  skipline(ifs);
  assert(c == '%' && n > 0);
  std::vector<unsigned int> r(n);
  while(!ifs.eof()) {
    unsigned int id;
    unsigned int hits;
    ifs >> id >> hits;
    unsigned int row = dataset.row_labels.index(id);
    r[row] = hits;
  }
  return r;
}

int main(int argc, char **argv) {

  if(argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <prefix> <f-test threshold> <hits threshold> [train_col1,train_col2,...]" << std::endl;
    exit(1);
  }

  MetabData metab_data(argv[1]);

  double threshold_p = std::atof(argv[2]);
  double hits_threshold = std::atoi(argv[3]);

  MetabData anova_data;
  if(threshold_p) {
    metab_data.calc_samples();
    metab_data.copy_anova(threshold_p, anova_data);
  } else {
    anova_data = metab_data;
  }

  std::vector<unsigned int> hits = readHits(argv[1], dataset);

  double threshold = 0;
  std::vector<double> ft;
  {
    int d1, d2;
    Dataset ft_dataset(dataset);
    // norm_sum(ft_dataset);
    ft = ftest(ft_dataset, d1, d2);
    if(d1 > 0 && d2 > 0)
      threshold = invfdistribution(d1, d2, std::abs(threshold_p));
  }
  std::cerr << "threshold = " << threshold << std::endl;

  std::vector<unsigned int> columns;
  for(unsigned int i=0; i < ft.size(); i++) {
    //    std::cout << ft[i] << "\t";
    if(((threshold == 0) ||
	(threshold_p > 0 && ft[i] > threshold) || 
	(threshold_p < 0 && ft[i] < threshold)) &&
       hits[i] > hits_threshold) {
      //      std::cerr << i << std::endl;
      columns.push_back(i);
    }
  }
  std::cerr << "Dimension after F-Test: " << columns.size() << std::endl;
  if(columns.size() == 0) {
    std::cerr << "No inputs left, exiting.\n";
    exit(0);
  }
  //  std::cout << std::endl;

  //  MeanDataset mean_dataset(dataset);
  Dataset subset;
  dataset.copy_columns(columns, subset);
  Dataset norm_subset(subset);
  autoscale_col(norm_subset);

  //  center_col(subset);

  subset.transpose();
  norm_subset.transpose();
  //  norm_level(subset);

  esom::ToroidGrid grid(100, 160, subset.dimension());
  const int epochs = 30;
  esom::distance::Correlation distance;
  esom::bestmatch::Linear bestmatch(distance);
  esom::neighbourhood::Gauss neighbourhood;
  esom::cooling::Linear cooling(.5, .1, epochs);
  esom::cooling::Linear radiusCooling(50, 1, epochs);
  esom::OnlineSOM som(grid, bestmatch, neighbourhood, cooling, radiusCooling);

  // srand(time(NULL));
  srand(1);
  som.init();

  for(int i = 0; i < epochs; i++) {
    som.train(norm_subset.data);
    som.endEpoch();
    std::cerr << "Epoch " << i << std::endl;
  }
  
  std::vector<int> bestmatches;
  for(unsigned int i=0; i < subset.inputs(); i++) {
    esom::Vector ve = norm_subset.data(i);
    bestmatches.push_back(bestmatch(ve));
  }

  esom::UMatrix um(distance, grid);
  um.calculate();

  writeUMatrix("umatrix.csv", um);

  esom::Watershed ws(um);
  // std::cerr << "bestmatches[" << bestmatches.size() << "]" << std::endl;
  // std::cerr << "bestmatches[16] = " << bestmatches[16] << std::endl;
  // std::cerr << "bestmatches[29] = " << bestmatches[29] << std::endl;
  esom::LabelTree tree = ws.tree(bestmatches);

  QApplication app(argc, argv);

  Dendrogram *den = new Dendrogram(tree, subset);

//  std::vector<unsigned int> column_order = read_column_order(argv[1]);
  std::vector<unsigned int> column_order(subset.dimension());
  for(unsigned int i=0; i < column_order.size(); i++) {
    column_order[i] = i;
  }

  VectorPlot *vp = new VectorPlot(subset, column_order);
  QObject::connect(den, SIGNAL(select(int)), vp, SLOT(displayRow(int)));

  UMatrixPlot *ump = new UMatrixPlot(subset, um, bestmatches);
  QObject::connect(ump, SIGNAL(select(int)), vp, SLOT(displayRow(int)));
  QObject::connect(ump, SIGNAL(select(int)), den, SLOT(activateItem(int)));

  HeatmapPlot *hmp = new HeatmapPlot(norm_subset, tree);
  QObject::connect(hmp, SIGNAL(select(int)), vp, SLOT(displayRow(int)));
  QObject::connect(hmp, SIGNAL(select(int)), den, SLOT(activateItem(int)));

  QTabWidget *tabs = new QTabWidget();
  tabs->addTab(ump, "UMatrix");
  tabs->addTab(den, "Dendrogram");
  tabs->addTab(hmp, "Heatmap");

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(tabs);
  layout->addWidget(vp);

  QWidget *window = new QWidget;

  window->setLayout(layout);
  //  window->show();

  QMainWindow *mainwindow = new QMainWindow;
  QToolBar *toolbar = mainwindow->addToolBar("Toolbar");
  QAction *printAct = new QAction("Print", mainwindow);
  toolbar->addAction(printAct);
  QObject::connect(printAct, SIGNAL(triggered()), den, SLOT(print()));

  QStatusBar *statusbar = mainwindow->statusBar();
  QObject::connect(vp, SIGNAL(legend(QString)), statusbar, SLOT(showMessage(QString)));
  QObject::connect(hmp, SIGNAL(legend(QString)), statusbar, SLOT(showMessage(QString)));

  mainwindow->setWindowTitle(argv[1]);
  mainwindow->setCentralWidget(window);
  mainwindow->addToolBar(Qt::TopToolBarArea, toolbar);
  mainwindow->show();

  return app.exec();
}
