#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cmath>

#include <errno.h>
#include <string.h>

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <qwt_panner.h>

#include <esom.h>

#include "stat.h"
#include "metabdata.h"
#include "metabcsv.h"

#include "dendrogram.h"
#include "vectorplot.h"
#include "vectorplotwidget.h"
#include "umatrixplot.h"
#include "heatmapplot.h"
#include "heatmapwidget.h"
#include "param_window.h"

// Uncomment this to enable floating point exceptions on NaN
#if 0
#define _GNU_SOURCE 1
#include <fenv.h>
static void __attribute__ ((constructor)) trapfpe(void)
{
  /* Enable some exceptions. At startup all exceptions are masked. */
  feenableexcept(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW);
}
#endif

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

int main(int argc, char **argv) {

  //  MetabData input_data2(argv[1]);
  //  input_data2.calc_all();
  
  QApplication app2(argc, argv);

  MetabData input_data2;
  const char *fname;
  std::string fname2;
  if(argc < 2) {
    //    QString filters("Tab-separated CSV (*.csv);;Databionics LRN (*.lrn);;All files (*)");
    QString filters("Tab-separated CSV (*.csv);;All files (*)");
    QString filename = QFileDialog::getOpenFileName(0, 
						    QString(), 
						    QString(),
						    filters
						    );
    if(filename.size() == 0) {
      exit(1);
    }
    fname2 = filename.toStdString();
    fname = fname2.c_str();
  } else {
    fname = argv[1];
  }
  CsvReader csv_reader(fname, input_data2.replicates);
  if(!csv_reader.is_open()) {
    std::stringstream ss;
    ss << "Unable to open " << fname << ": " << strerror(errno);
    QMessageBox::critical(0, "File open failed", ss.str().c_str());
    exit(1);
  }    
  if(!csv_reader.readMagic()) {
    std::stringstream ss;
    ss << "Error while reading from " << fname;
    QMessageBox::critical(0, "File reading failed", ss.str().c_str());
    exit(1);
  }
  input_data2.replicates.transpose();
  input_data2.calc_hits();
  std::cerr << "Data rows: " << input_data2.replicates.row_labels.size() << std::endl;
  
  ParamWindow *w = new ParamWindow();
  w->load(input_data2);
  w->show();
  return app2.exec();
}
