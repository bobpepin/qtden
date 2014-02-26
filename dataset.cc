#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cassert>
#include <iterator>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>

#include <esom.h>

#include "dataset.h"

Labels::Labels(const unsigned int n) {
  resize(n);
  group_ids.resize(1);
  group_ids[0] = 0;
}

std::vector<std::vector<unsigned int> > Labels::bins() const {
  std::vector<std::vector<unsigned int> > bins;
  for(unsigned int i=0; i < groups.size(); i++) {
    unsigned int gid = groups[i];
    if(bins.size() < gid+1) {
      bins.resize(gid+1);
    }
    bins[gid].push_back(i);
  }
  return bins;
}

void Labels::resize(unsigned int n) {
  unsigned int oldsize = size();
  names.resize(n);
  groups.resize(n);
  ids.resize(n);
  for(unsigned int i=oldsize; i < n; i++) {
    ids[i] = i;
  }
}

int Labels::index(unsigned int id) const {
  for(unsigned int i=0; i < ids.size(); i++) {
    if(ids[i] == id) {
      return i;
    }
  }
  return -1;
}

int Labels::group_index(unsigned int id) const {
  for(unsigned int i=0; i < group_ids.size(); i++) {
    if(group_ids[i] == id) {
      return i;
    }
  }
  return -1;
}

unsigned int Labels::countGroups() const {
  return group_ids.size();
}

void Labels::setGroups(const std::map<unsigned int, unsigned int>& group_map) {
  std::multimap<unsigned int, unsigned int> group_members;
  for(std::map<unsigned int, unsigned int>::const_iterator iter = group_map.begin();
      iter != group_map.end(); iter++) {
    unsigned int index = iter->first;
    unsigned int groupid = iter->second;
    group_members.insert(std::make_pair(groupid, index));
  }

  typedef std::multimap<unsigned int, unsigned int>::const_iterator CI;
  CI iter = group_members.begin();
  unsigned int last_groupid = iter->first;
  unsigned int group_index = 0;
  group_ids.resize(group_index+1);
  group_ids[group_index] = last_groupid;
  for(; iter != group_members.end(); iter++) {
    unsigned int groupid = iter->first;
    unsigned int index = iter->second;
    if(groupid != last_groupid) {
      group_index++;
      last_groupid = groupid;
      group_ids.resize(group_index+1);
      group_ids[group_index] = last_groupid;
    }
    if(size() < index+1) {
      resize(index+1);
    }
    groups[index] = group_index;
  }
}

void Labels::setGroup(unsigned int row, unsigned int gid) {
  int group = group_index(gid);
  if(group == -1) {
    group = countGroups();
    group_ids.resize(group+1);
    group_ids[group] = gid;
  }
  groups[row] = group;
}

#if 0  
  std::map<unsigned int, unsigned int> group_ids;
  unsigned int next_group = 0;
  for( iter = group_map.begin();
      iter != group_map.end(); iter++) {
    unsigned int index = iter->first;
    unsigned int groupid = iter->second;
    if(!group_ids.count(groupid)) {
      group_ids[groupid] = next_group;
      next_group++;
    }
    int group = group_ids[groupid];
    if(size() < index+1) {
      resize(index+1);
    }
    groups[index] = group;
  }
#endif

void Labels::setNames(const std::map<unsigned int, std::string>& name_map) {
  for(std::map<unsigned int, std::string>::const_iterator iter = name_map.begin();
      iter != name_map.end(); iter++) {
    unsigned int index = iter->first;
    std::string name = iter->second;
    if(size() < index+1) {
      resize(index+1);
    }
    names[index] = name;
  }
}

void Labels::setIdGroups(const std::map<unsigned int, unsigned int>& group_map) {
  std::map<unsigned int, unsigned int> row_groups;
  for(std::map<unsigned int, unsigned int>::const_iterator iter = group_map.begin();
      iter != group_map.end(); iter++) {
    unsigned int id = iter->first;
    unsigned int groupid = iter->second;
    unsigned int row = this->index(id);
    row_groups[row] = groupid;
  }
  setGroups(row_groups);
}

void Labels::setIdNames(const std::map<unsigned int, std::string>& name_map) {
  for(std::map<unsigned int, std::string>::const_iterator iter = name_map.begin();
      iter != name_map.end(); iter++) {
    int id = iter->first;
    std::string name = iter->second;
    int row = this->index(id);
    if(size() < row+1) {
      resize(row+1);
    }
    //    std::cerr << row << " " << id << " -> " << name << std::endl;
    names[row] = name;
  }
}

void Labels::copy_subseq(const std::vector<unsigned int>& indices,
			 Labels& out) const {
  out.resize(indices.size());
  out.group_ids.resize(0);
  for(unsigned int j=0; j < indices.size(); j++) {
    out.names[j] = names[indices[j]];
    //    out.groups[j] = groups[indices[j]];
    out.ids[j] = ids[indices[j]];
    out.setGroup(j, group_id(groups[indices[j]]));
  }
  //#warning "FIXME"
  //  out.group_ids = group_ids;
}

Dataset::Dataset() : p(0), data(0, 0, 0) {}

Dataset::Dataset(esom::Matrix data) : p(0), data(data) {
  row_labels.resize(data.rows());
  column_labels.resize(data.columns());
}

void Dataset::resize(unsigned int rows, unsigned int cols) {
  if(this->rows() == rows && this->columns() == cols) {
    return;
  }
  if(p) {
    delete[] p;
  }
  p = new double[rows*cols];
  data = esom::Matrix(p, rows, cols);
  row_labels.resize(rows);
  column_labels.resize(cols);
}

Dataset::Dataset(const Dataset& dataset) : p(0), data(0, 0, 0) {
  row_labels = dataset.row_labels;
  column_labels = dataset.column_labels;
  p = new double[dataset.inputs() * dataset.dimension()];
  std::memcpy(p, dataset.p, dataset.rows()*dataset.columns()*sizeof(double));
  data = esom::Matrix(p, dataset.rows(), dataset.columns());
}

Dataset& Dataset::operator=(const Dataset& dataset) {
  if(this != &dataset) {
    row_labels = dataset.row_labels;
    column_labels = dataset.column_labels;
    resize(dataset.rows(), dataset.columns());
    std::memcpy(p, dataset.p, dataset.rows()*dataset.columns()*sizeof(double));
  }
  return *this;
}

Dataset::~Dataset() {
  if(p) {
    delete[] p;
  }
}

void Dataset::copy_columns(const std::vector<unsigned int>& indices, 
			   Dataset& out) const {
  out.row_labels = row_labels;
  if(column_labels.size() != 0) {
    column_labels.copy_subseq(indices, out.column_labels);
  }

  if(!empty()) {
    out.resize(rows(), indices.size());

    for(unsigned int i=0; i < inputs(); i++) {
      for(unsigned int j=0; j < indices.size(); j++) {
	out.data(i, j) = data(i, indices[j]);
      }
    }
  }
}

void Dataset::copy_rows(const std::vector<unsigned int>& indices, 
			Dataset& out) const {
  out.column_labels = column_labels;
  if(row_labels.size() != 0) {
    row_labels.copy_subseq(indices, out.row_labels);
  }

  if(!empty()) {
    out.resize(indices.size(), columns());

    for(unsigned int i=0; i < indices.size(); i++) {
      out.data(i) = data(indices[i]);
    }
  }
}

    
void Dataset::transpose() {
  Labels tmp_labels = column_labels;
  column_labels = row_labels;
  row_labels = tmp_labels;
  //      std::cerr << "names[0] " << names[0] << std::endl;
  double *tmp_data = new double[data.rows() * data.columns()];
  esom::Matrix tmp(tmp_data, data.columns(), data.rows());
  for(unsigned int i=0; i < data.rows(); i++) {
    for(unsigned int j=0; j < data.columns(); j++) {
      tmp(j, i) = data(i, j);
    }
  }
  delete[] p;
  p = tmp_data;
  data = tmp;
}

#if 0
StatDataset::StatDataset() : p_tstat(0), data_tstat(0, 0, 0), Dataset() {}
StatDataset::StatDataset(esom::Matrix data) 
  : p_tstat(0), data_tstat(0, 0, 0), Dataset(data) {}
StatDataset::StatDataset(const Dataset& dataset)
  : p_stat(0), data_tstat(0, 0, 0), Dataset(dataset) {}
StatDataset::StatDataset(const StatDataset& stdataset)
  : p_stat(0), data_stat(0, 0, 0), Dataset(stdataset) {
  p_tstat = new double[dataset.inputs() * dataset.dimension()];
  std::memcpy(p_tstat, stdataset.p_tstat, 
	      stdataset.rows()*stdataset.columns()*sizeof(double));
  data_tstat = esom::Matrix(p_tstat, dataset.rows(), dataset.columns());
}

StatDataset& StatDataset::operator=(const StatDataset& stdataset) {
  Dataset::operator=(stdataset);
  if(this != &stdataset) {
    std::memcpy(p_tstat, stdataset.p_tstat, 
		stdataset.rows()*stdataset.columns()*sizeof(double));
  }
  return *this;
}

StatDataset& StatDataset::resize(unsigned int rows, unsigned int columns) {
  if(this->rows() == rows && this->cols() == cols) {
    return;
  }
  if(p_tstat) {
    delete[] p_tstat;
  }
  p_tstat = new double[rows*cols];
  data_tstat = esom::Matrix(p_tstat, rows, cols);
}

void StatDataset::transpose() {
  Dataset::transpose();
  double *tmp_data = new double[data.rows() * data.columns()];
  esom::Matrix tmp(tmp_data, data.columns(), data.rows());
  for(unsigned int i=0; i < data_tstat.rows(); i++) {
    for(unsigned int j=0; j < data_tstat.columns(); j++) {
      tmp(j, i) = data_tstat(i, j);
    }
  }
  delete[] p_tstat;
  p = tmp_data;
  data_tstat = tmp;
}
#endif
