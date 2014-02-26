#include <cfloat>

#include "metabdata.h"
#include "lrn.h"
#include "stat.h"

extern "C" {
  double fdtri(int, int, double);
}

MetabData::MetabData(std::string prefix) {
  loadLrn(prefix);
}

MetabData::MetabData() : samples_dimension(None) {}

void MetabData::loadLrn(std::string prefix) {
  std::string lrnFile = prefix + ".lrn";
  std::string namesFile = prefix + ".names";
  std::string metabGroupsFile = prefix + ".metcls";
  std::string repGroupsFile = prefix + ".repcls";
  std::string sampleGroupsFile = prefix + ".samcls";
  std::string sampleNamesFile = prefix + ".samnames";
  std::string metabHitsFile = prefix + ".hits";

  std::ifstream ifs;

  {
    int inputs;
    int dimension;
    std::vector<int> column_types;
    ifs.open(lrnFile.c_str());
    assert(ifs.is_open());
    readLrnHeader(ifs, inputs, dimension, column_types, replicates.column_labels);
    std::cerr << inputs << " inputs, dimension " << dimension << std::endl;
    replicates.resize(inputs, dimension);
    readLrnData(ifs, replicates.data, column_types, replicates.row_labels);
    ifs.close();
  }

  ifs.open(namesFile.c_str());
  if(ifs.is_open()) {
    std::map<unsigned int, std::string> names;
    readLrnMap(ifs, names);
    replicates.row_labels.setIdNames(names);
    samples.row_labels = replicates.row_labels;
    ifs.close();
    std::cerr << "Read metabolite names." << std::endl;
  }

  ifs.open(metabGroupsFile.c_str());
  if(ifs.is_open()) {
    std::map<unsigned int, unsigned int> groups;
    readLrnMap(ifs, groups);
    replicates.row_labels.setIdGroups(groups);
    ifs.close();
    std::cerr << "Read metabolite groups." << std::endl;
  }

  ifs.open(repGroupsFile.c_str());
  if(ifs.is_open()) {
    std::map<unsigned int, unsigned int> groups;
    readLrnMap(ifs, groups);
    replicates.column_labels.setGroups(groups);
    ifs.close();
    std::cerr << "Read " << replicates.column_labels.countGroups()
	      << " replicate groups." << std::endl;
  }

  ifs.open(sampleGroupsFile.c_str());
  if(ifs.is_open()) {
    std::map<unsigned int, unsigned int> groups;
    readLrnMap(ifs, groups);
    samples.column_labels.setGroups(groups);
    ifs.close();
    std::cerr << "Read " << samples.column_labels.countGroups() 
	      << " sample groups." << std::endl;
  }

  ifs.open(sampleNamesFile.c_str());
  if(ifs.is_open()) {
    std::map<unsigned int, std::string> names;
    readLrnMap(ifs, names);
    samples.column_labels.setIdNames(names);
    ifs.close();
    std::cerr << "Read sample names." << std::endl;
  }

  ifs.open(metabHitsFile.c_str());
  if(ifs.is_open()) {
    std::map<unsigned int, unsigned int> hits_map;
    readLrnMap(ifs, hits_map);
    for(std::map<unsigned int, unsigned int>::const_iterator iter = hits_map.begin();
      iter != hits_map.end(); iter++) {
      unsigned int id = iter->first;
      unsigned int count = iter->second;
      unsigned int row = replicates.row_labels.index(id);
      if(hits.size() < row+1) {
	hits.resize(row+1);
      }
      hits[row] = count;
    }
    ifs.close();
    std::cerr << "Read hits file." << std::endl;
  }
}

unsigned int MetabData::rows() const {
  return replicates.rows();
}

void MetabData::calc_hits() {
  hits.resize(replicates.rows());
  for(unsigned int i=0; i < replicates.rows(); i++) {
    unsigned int sum = 0;
    for(unsigned int j=0; j < replicates.columns(); j++) {
      if(replicates.data(i, j) != 0) {
	sum++;
      }
    }
    hits[i] = sum;
  }
}

void MetabData::calc_replicates_tstat() {
  replicates_tstat.row_labels = replicates.row_labels;
  replicates_tstat.column_labels = replicates.column_labels;
  replicates_tstat.resize(replicates.rows(), replicates.columns());
  stat::tstat(replicates.data, replicates_tstat.data);
}

void MetabData::calc_samples() {
  samples.row_labels = replicates.row_labels;
  std::vector< std::vector<unsigned int> > groups 
    = replicates.column_labels.bins();
  samples.resize(replicates.rows(), samples.column_labels.size());
  for(unsigned int k=0; k < groups.size(); k++) {
    unsigned int gid = replicates.column_labels.group_id(k);
    int column = samples.column_labels.index(gid);
    if(column != -1) {
      for(unsigned int i=0; i < samples.rows(); i++) {
	samples.data(i, column) = stat::mean_g(replicates.data(i), groups[k]);
      }
    }
  }
  samples_dimension = Columns;
}

void MetabData::calc_samples_tstat() {
  samples_tstat.row_labels = samples.row_labels;
  samples_tstat.column_labels = samples.column_labels;
  samples_tstat.resize(samples.rows(), samples.columns());
  stat::tstat(samples.data, samples_tstat.data);
}

void MetabData::calc_samples_std() {
  samples_std.row_labels = samples.row_labels;
  samples_std.column_labels = samples.column_labels;
  samples_std.resize(samples.rows(), samples.columns());
  std::vector< std::vector<unsigned int> > groups 
    = replicates.column_labels.bins();
  for(unsigned int k=0; k < groups.size(); k++) {
    for(unsigned int i=0; i < samples_std.rows(); i++) {
      unsigned int gid = replicates.column_labels.group_id(k);
      int column = samples.column_labels.index(gid);
      if(column != -1) {
	double m = stat::mean_g(replicates.data(i), groups[k]);
	samples_std.data(i, column) = stat::std_g(replicates.data(i), groups[k], m);
      }
    }
  }
}

void MetabData::calc_samples_cv() {
  samples_cv.row_labels = samples.row_labels;
  samples_cv.column_labels = samples.column_labels;
  samples_cv.resize(samples.rows(), samples.columns());
  for(unsigned int i=0; i < samples_std.rows(); i++) {
    samples_cv.data(i) = samples_std.data(i);
    for(unsigned int j=0; j < samples.columns(); j++) {
      double v = samples.data(i, j);
      if(v == 0) {
	v = DBL_MIN;
      }
      samples_cv.data(i, j) /= v;
    }
  }
}

void MetabData::calc_all() {
  calc_hits();
  calc_replicates_tstat();
  calc_samples();
  calc_samples_tstat();
  calc_samples_std();
  calc_samples_cv();
}

void MetabData::copy_rows(std::vector<unsigned int> rows, MetabData& out) {
  replicates.copy_rows(rows, out.replicates);
  replicates_tstat.copy_rows(rows, out.replicates_tstat);
  samples.copy_rows(rows, out.samples);
  samples_tstat.copy_rows(rows, out.samples_tstat);
  samples_std.copy_rows(rows, out.samples_std);
  out.samples_dimension = samples_dimension;
  out.hits.resize(rows.size());
  for(unsigned int i=0; i < rows.size(); i++) {
    out.hits[i] = hits[rows[i]];
  }
}

void MetabData::transpose() {
  replicates.transpose();
  replicates_tstat.transpose();
  samples.transpose();
  samples_tstat.transpose();
  samples_std.transpose();
  samples_cv.transpose();
  if(samples_dimension == Rows) {
    samples_dimension = Columns;
  } else if(samples_dimension == Columns) {
    samples_dimension = Rows;
  }
}

std::vector<unsigned int> anova_test(const Dataset& dataset, double p) {
  std::vector<std::vector<unsigned int> > groups = dataset.column_labels.bins();
  unsigned int d1 = groups.size() - 1;
  unsigned int d2 = dataset.columns() - groups.size();
  //  std::cerr << "d1 = " << d1 << " d2 = " << d2 << std::endl;
//  double threshold = gsl_cdf_fdist_Qinv(p, d1, d2);
  double threshold = fdtri(d1, d2, p);
  std::vector<unsigned int> rows;
  for(unsigned int i=0; i < dataset.rows(); i++) {
    if(stat::anova_ftest(dataset.data(i), groups) >= threshold) {
      rows.push_back(i);
    }
  }
  return rows;
}

std::vector<unsigned int> hits_test(const MetabData& metab_data, 
				    unsigned int n) {
  std::vector<unsigned int> rows;
  for(unsigned int i=0; i < metab_data.hits.size(); i++) {
    if(metab_data.hits[i] >= n) {
      rows.push_back(i);
    }
  }
  return rows;
}
