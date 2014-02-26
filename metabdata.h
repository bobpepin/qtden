#ifndef QTDEN_METABDATA_H
#define QTDEN_METABDATA_H

#include <vector>
#include <string>
#include "dataset.h"

class MetabData {
 public:
  enum { None, Rows, Columns } samples_dimension;
  Dataset replicates;
  Dataset replicates_tstat;
  Dataset samples;
  Dataset samples_tstat;
  Dataset samples_std;
  Dataset samples_cv;
  std::vector<unsigned int> hits;
  MetabData();
  MetabData(std::string prefix);
  void loadLrn(std::string prefix);
  void calc_hits();
  void calc_replicates_tstat();
  void calc_samples();
  void calc_samples_tstat();
  void calc_samples_std();
  void calc_samples_cv();
  void calc_all();
  unsigned int rows() const;
  void copy_rows(std::vector<unsigned int> rows, MetabData& out);
  void transpose();
};

std::vector<unsigned int> anova_test(const Dataset& data, double p);
std::vector<unsigned int> hits_test(const MetabData&, unsigned int n);

#endif
