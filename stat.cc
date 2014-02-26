#include <vector>
#include <cfloat>
//#include <gsl/gsl_statistics_double.h>
#include "stat.h"

double stat::mean(const esom::Vector x) {
  double s = 0;
  for(unsigned int i=0; i < x.size(); i++) {
    s += x[i];
  }
  return s / x.size();
//  return gsl_stats_mean(x, 1, x.size());
}

double stat::mean_g(const esom::Vector x, 
			  const std::vector<unsigned int>& g) {
  double group_mean = 0;
  for(unsigned int k=0; k < g.size(); k++) {
    group_mean += x[g[k]];
  }
  return (group_mean / g.size());
}

double stat::std(const esom::Vector x, double m) {
//  return gsl_stats_sd_m(x, 1, x.size(), m);
  double s = 0;
  for(unsigned int i=0; i < x.size(); i++) {
    s += std::pow(x[i] - m, 2);
  }
  unsigned int n = x.size();
  return std::sqrt(s / (n==1 ? 1 : n-1));
}

double stat::std_g(const esom::Vector x, 
			 const std::vector<unsigned int>& g,
			 double m) {
  double group_std = 0;
  for(unsigned int k=0; k < g.size(); k++) {
    group_std += std::pow(x[g[k]] - m, 2);
  }
  unsigned int n = g.size();
  return std::sqrt(group_std / (n==1 ? 1 : n-1));
}

void stat::tstat(const esom::Matrix data, esom::Matrix out) {
  for(unsigned int i=0; i < data.rows(); i++) {
    double mean_val = mean(data(i));
    double std_val = std(data(i), mean_val);
    out(i) = data(i);
    out(i) -= mean_val;
    if(std_val == 0) {
      std_val = DBL_MIN;
    }
    out(i) /= std_val;
  }
}

double stat::anova_ftest(const esom::Vector x,
			 const std::vector< std::vector<unsigned int> >& groups)
{
  double mean_v = mean(x);
  unsigned int K = groups.size();
  unsigned int N = x.size();
  double sstr = 0; // Sum Squares Treatment
  double sse = 0; // Sum Squares Error
  for(unsigned int i=0; i < K; i++) {
    std::vector<unsigned int> idx = groups[i];
    double group_mean = mean_g(x, idx);
    unsigned int n = idx.size();
    sstr += n*std::pow((group_mean - mean_v), 2);
    for(unsigned int l=0; l < n; l++) {
      unsigned int j = idx[l];
      sse += std::pow(x[j] - group_mean, 2);
    }
  }
  return (sstr*(N-K)) / (sse*(K-1));
}
