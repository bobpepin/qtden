#include "meandataset.h"

#include "stat.h"

MeanDataset::MeanDataset(const Dataset& dataset) : var(0, 0, 0) {
  unsigned int inputs = dataset.row_labels.countGroups();
  unsigned int dim = dataset.dimension();
  resize(inputs, dim);
  var_data = new double[dim*inputs];
  var = esom::Matrix(var_data, inputs, dim);
  std::vector< std::vector<unsigned int> > bins = dataset.row_labels.bins();
  std::vector<unsigned int> groups(inputs);
  std::vector<unsigned int> ids(inputs);
  std::vector<std::string> names(inputs);
  for(unsigned int k=0; k < bins.size(); k++) {
    mean_g(dataset.data, bins[k], data(k));
    ssq_g(dataset.data, data(k), bins[k], var(k));
    unsigned int n = bins[k].size();
    if(n > 1) {
      var(k) /= (n - 1);
    }
    groups[k] = dataset.row_labels.group(bins[k][0]);
    names[k] = dataset.row_labels.name(bins[k][0]);
    ids[k] = dataset.row_labels.id(bins[k][0]);
  }
  row_labels.setIds(ids);
  row_labels.setNames(names);
  row_labels.setGroups(groups);
  column_labels = dataset.column_labels;
}

MeanDataset::~MeanDataset() {
  delete[] var_data;
}

void MeanDataset::transpose() {
  Dataset::transpose();
  double *tmp_data = new double[var.rows() * var.columns()];
  esom::Matrix tmp(tmp_data, var.columns(), var.rows());
  for(unsigned int i=0; i < var.rows(); i++) {
    for(unsigned int j=0; j < var.columns(); j++) {
      tmp(j, i) = var(i, j);
    }
  }
  delete[] var_data;
  var_data = tmp_data;
  var = tmp;
}
