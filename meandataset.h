#ifndef QTDEN_MEAN_DATASET
#define QTDEN_MEAN_DATASET

#include <esom.h>
#include "dataset.h"

class MeanDataset : public Dataset {
 private:
  double *var_data;
 public:
  esom::Matrix var;
  MeanDataset(const Dataset&);
  ~MeanDataset();
  void transpose();
};

#endif
