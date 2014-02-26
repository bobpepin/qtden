#include <esom.h>

#include "dataset.h"

void mean_g(esom::Matrix mat, std::vector<unsigned int> indices, 
	    esom::Vector out) {
  out = 0;
  for(unsigned int i=0; i < indices.size(); i++) {
    out += mat(indices[i]);
  }
  out /= indices.size();
}

void ssq_g(esom::Matrix mat, esom::Vector mean, 
	   std::vector<unsigned int> indices, esom::Vector out) {
  double tmp_data[mean.size()];
  esom::Vector tmp(tmp_data, mean.size());
  out = 0;
  for(unsigned int i=0; i < indices.size(); i++) {
    tmp = mat(indices[i]);
    //    std::cout << tmp << std::endl;
    tmp -= mean;
    //    std::cout << tmp << std::endl;
    tmp.pow(2);
    //    std::cout << tmp << std::endl;
    out += tmp;
  }
}

void mean_col(esom::Matrix data, esom::Vector out) {
  out = 0;
  for(unsigned int i=0; i < data.rows(); i++) {
    out += data(i);
  }
  out /= data.rows();
}

double mean(esom::Vector v) {
  double r = 0;
  for(unsigned int i=0; i < v.size(); i++) {
    r += v[i];
  }
  return r / (double)v.size();
}

void ssq_col(esom::Matrix data, esom::Vector mean, esom::Vector out) {
  double tmp_data[mean.size()];
  esom::Vector tmp(tmp_data, mean.size());
  out = 0;
  for(unsigned int i=0; i < data.rows(); i++) {
    tmp = data(i);
    tmp -= mean;
    tmp.pow(2);
    out += tmp;
    //    std::cerr << "out: " << out << std::endl;
  }
}

void norm_level(Dataset& dataset) {
  esom::Matrix data = dataset.data;
  int inputs = data.rows();
  int dim = data.columns();
  for(int i=0; i < inputs; i++) {
    int mean = 0;
    for(int j=0; j < dim; j++) {
      mean += data(i, j);
    }
    if(mean == 0) continue;
    mean /= dim;
    for(int j=0; j < dim; j++) {
      data(i, j) -= mean;
      data(i, j) /= mean;
    }
  }
}

void norm_level_col(Dataset& dataset) {
  esom::Matrix data = dataset.data;
  int inputs = data.rows();
  int dim = data.columns();
  for(int j=0; j < dim; j++) {
    int mean = 0;
    for(int i=0; i < inputs; i++) {
      mean += data(i, j);
    }
    if(mean == 0) continue;
    mean /= inputs;
    for(int i=0; i < inputs; i++) {
      data(i, j) -= mean;
      data(i, j) /= mean;
    }
  }
}

void autoscale_col(Dataset& dataset) {
  esom::Matrix data = dataset.data;
  int inputs = data.rows();
  int dim = data.columns();
  double mean_data[dim];
  esom::Vector mean_v(mean_data, dim);
  double std_data[dim];
  esom::Vector std_v(std_data, dim);
  mean_col(data, mean_v);
  ssq_col(data, mean_v, std_v);
  std_v.pow(0.5);
  //  std::cerr << "ssq: " << std_v << std::endl;
  std_v /= (inputs == 1) ? 1 : (inputs - 1);
  //  std::cerr << "mean: " << mean_v << std::endl;
  //  std::cerr << "stddev: " << std_v << std::endl;
  for(int i=0; i < inputs; i++) {
    data(i) -= mean_v;
    data(i) /= std_v;
  }
}

double norm2(esom::Vector v) {
  double r = 0;
  for(unsigned int i=0; i < v.size(); i++) {
    r += v[i]*v[i];
  }
  return r;
}

void norm_group_mean(Dataset& dataset) {
  std::vector< std::vector<unsigned int> > bins = dataset.row_labels.bins();
  double mean_data[dataset.dimension()];
  esom::Matrix& data = dataset.data;
  esom::Vector mean_v(mean_data, dataset.dimension());
  for(unsigned int k=0; k < bins.size(); k++) {
    std::vector<unsigned int> range = bins[k];
    mean_g(data, range, mean_v);
    double mean_norm2 = norm2(mean_v);
    for(unsigned int r=0; r < range.size(); r++) {
      unsigned int i = range[r];
      double n2 = norm2(data(i));
      //      std::cerr << "mean norm = " << sqrt(mean_norm2) << " norm = " << sqrt(n2) << std::endl;
      data(i) *= (sqrt(mean_norm2 / n2));
      //      std::cerr << "norm(" << i << ") = " << sqrt(norm2(data(i))) << std::endl;
    }
  }
}

std::vector<double> inGroupStd(Dataset& dataset) {
  unsigned int ngroups = dataset.row_labels.countGroups();
  unsigned int dim = dataset.dimension();
  esom::Matrix data = dataset.data;

  unsigned int counts[ngroups];
  std::fill_n(counts, ngroups, 0);
  // Calculate the mean of columns j for group i in mean(i, j)
  // Counts elements in each group at the same time
  double mean_data[ngroups * dim];
  std::fill_n(mean_data, ngroups*dim, 0);
  esom::Matrix mean(mean_data, ngroups, dim);
  for(unsigned int i=0; i < dataset.inputs(); i++) {
    esom::Vector v = data(i);
    int g = dataset.row_labels.group(i);
    mean(g) += v;
    counts[g]++;
  }
  for(unsigned int g=0; g < ngroups; g++) {
    mean(g) /= counts[g];
  }

  // Calculate the sample standard deviation std(i, j) for each group and column
  double std_data[ngroups * dim];
  std::fill_n(std_data, ngroups*dim, 0);
  esom::Matrix std(std_data, ngroups, dim);
  for(unsigned int i=0; i < dataset.inputs(); i++) {
    unsigned int g = dataset.row_labels.group(i);
    for(unsigned int j=0; j < dim; j++) {
      std(g, j) += pow(data(i, j) - mean(g, j), 2);
    }
  }
  for(unsigned int g=0; g < ngroups; g++) {
    if(counts[g] > 1) {
      std(g) /= counts[g] - 1;
    } else {
      std(g) = 0;
    }
  }

  std::vector<double> maxstd(dim);
  for(unsigned int j=0; j < dim; j++) {
    double m = 0;
    for(unsigned int g=0; g < ngroups; g++) {
      double rd = sqrt(std(g, j)) / mean(g, j);
      if(rd > m) {
	m = rd;
      }
    }
    maxstd[j] = m;
  }

  return maxstd;
}

std::vector<double> ftest(Dataset& dataset, int& d1, int& d2) {
  esom::Matrix data = dataset.data;

  double mean_data[dataset.dimension()];
  esom::Vector mean(mean_data, dataset.dimension());
  mean = 0;
  for(unsigned int i=0; i < dataset.data.rows(); i++) {
    mean += data(i);
  }
  mean /= data.rows();

  double sstr_data[dataset.dimension()]; // Sum Squares Treatment
  esom::Vector sstr(sstr_data, dataset.dimension());
  sstr = 0;
  double sse_data[dataset.dimension()]; // Sum Squares Error
  esom::Vector sse(sse_data, dataset.dimension());
  sse = 0;

  std::vector<std::vector<unsigned int> > gbins = dataset.row_labels.bins();
  std::cerr << gbins.size() << " groups" << std::endl;
  d1 = gbins.size() - 1;
  d2 = (dataset.inputs() - gbins.size());
  std::cerr << "F-distribution parameters: d1 = " << d1
	    << " d2 = " << d2 << std::endl;
  double gmean_data[dataset.dimension()];
  esom::Vector gmean(gmean_data, dataset.dimension());
  double ssq_data[dataset.dimension()];
  esom::Vector ssq(ssq_data, dataset.dimension());
  for(unsigned int i=0; i < gbins.size(); i++) {
    mean_g(data, gbins[i], gmean);
    ssq_g(data, gmean, gbins[i], ssq);
    gmean -= mean;
    gmean.pow(2);
    gmean *= gbins[i].size();
    sstr += gmean;
    sse += ssq;
  }
  //  std::cerr << sstr[91] << std::endl;
  sstr /= (gbins.size() - 1);
  sse /= (dataset.inputs() - gbins.size());

  std::vector<double> f;
  for(unsigned int i=0; i < dataset.dimension(); i++) {
    f.push_back(sstr[i] / sse[i]);
  }
  return f;
}

void norm_mean_col(Dataset& dataset) {
  double mean_data[dataset.dimension()];
  esom::Vector mean_v(mean_data, dataset.dimension());
  mean(dataset.data, mean_v);
  for(unsigned int i=0; i < dataset.inputs(); i++) {
    dataset.data(i) /= mean_v;
  }
}

void center_col(Dataset& dataset) {
  double mean_data[dataset.dimension()];
  esom::Vector mean_v(mean_data, dataset.dimension());
  mean(dataset.data, mean_v);
  for(unsigned int i=0; i < dataset.inputs(); i++) {
    dataset.data(i) -= mean_v;
  }
}

void norm_sum(Dataset& dataset) {
  esom::Matrix& data = dataset.data;
  for(unsigned int i=0; i < data.rows(); i++) {
    double sum = 0;
    for(unsigned int j=0; j < data.columns(); j++) {
      sum += data(i, j);
    }
    data(i) /= sum;
  }
}

void sum(Dataset& dataset, esom::Vector out) {
  esom::Matrix& data = dataset.data;
  for(unsigned int i=0; i < data.rows(); i++) {
    double sum = 0;
    for(unsigned int j=0; j < data.columns(); j++) {
      sum += data(i, j);
    }
    out[i] = sum;
  }
}

void norm_factors(Dataset& dataset, 
		  std::vector<unsigned int> inputs,
		  esom::Vector out) {
  double v_data[dataset.dimension()];
  esom::Vector v(v_data, dataset.dimension());
  out = 0;
  for(unsigned int k=0; k < inputs.size(); k++) {
    unsigned int i = inputs[k];
    v = dataset.data(i);
    v /= mean(v);
    out += v;
  }
  out /= inputs.size();
}
