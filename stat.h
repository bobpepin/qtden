#ifndef QTDEN_STAT_H
#define QTDEN_STAT_H

#include "dataset.h"

namespace stat {
  double mean(const esom::Vector v);
  double mean_g(const esom::Vector v, const std::vector<unsigned int>& g);
  double std(const esom::Vector x, double m);
  double std_g(const esom::Vector v, const std::vector<unsigned int>& g, 
	       double m);
  void tstat(const esom::Matrix data, esom::Matrix out);
  double anova_ftest(const esom::Vector x,
		     const std::vector< std::vector<unsigned int> >& groups);
}

#endif
