#ifndef QTDEN_STAT_H
#define QTDEN_STAT_H

#include "dataset.h"

void mean_g(esom::Matrix mat, std::vector<unsigned int> indices, 
	    esom::Vector out);
void ssq_g(esom::Matrix mat, esom::Vector mean, 
	   std::vector<unsigned int> indices, esom::Vector out);
void mean(esom::Matrix data, esom::Vector out);
double mean(esom::Vector v);
void ssq(esom::Matrix data, esom::Vector mean, esom::Vector out);
void norm_level(Dataset& dataset);
void norm_level_col(Dataset& dataset);
void autoscale_col(Dataset& dataset);
void norm_group_mean(Dataset& dataset);
std::vector<double> inGroupStd(Dataset& dataset);
std::vector<double> ftest(Dataset& dataset, int& d1, int& d2);
void norm_mean_col(Dataset& dataset);
void center_col(Dataset& dataset);
void norm_sum(Dataset& dataset);
void sum(Dataset& dataset, esom::Vector out);
void norm_factors(Dataset& dataset, std::vector<unsigned int>, esom::Vector);

#endif
