#ifndef QTDEN_DATASET_H
#define QTDEN_DATASET_H

#include <map>
#include <esom.h>

class Labels {
 private:
  std::vector<unsigned int> groups;
  std::vector<unsigned int> group_ids;
  std::vector<std::string> names;
  std::vector<unsigned int> ids;
 public:
  Labels(const unsigned int n = 0);
  unsigned int size() const { return names.size(); }
  void resize(unsigned int n);
  void setNames(const std::vector<std::string>& names_v) {
    resize(names_v.size());
    names = names_v;
  }
  void setIds(const std::vector<unsigned int>& ids_v) {
    resize(ids_v.size());
    ids = ids_v;
  }
  void setGroups(const std::vector<unsigned int>& groups_v) {
    resize(groups_v.size());
    groups = groups_v;
  }
  void setNames(const std::map<unsigned int, std::string>& name_map);
  void setGroups(const std::map<unsigned int, unsigned int>& group_map);
  void setGroup(unsigned int row, unsigned int gid);
  void setName(unsigned int index, std::string name) {
    resize(index+1);
    names[index] = name;
  }
  void setIdNames(const std::map<unsigned int, std::string>& id_name_map);
  void setIdGroups(const std::map<unsigned int, unsigned int>& id_group_map);
  std::vector<std::vector<unsigned int> > bins() const;
  unsigned int id(unsigned int row) const { return ids[row]; }
  unsigned int group(unsigned int row) const { return groups[row]; }
  std::string name(unsigned int row) const { return names[row]; }
  int index(unsigned int id) const;
  unsigned int group_id(unsigned int group) const { return group_ids[group]; }
  int group_index(unsigned int gid) const;
  unsigned int countGroups() const;
  void copy_subseq(const std::vector<unsigned int>& indices, Labels& out) const;
};

class Dataset {
  double *p;

 public:
  Labels column_labels;
  Labels row_labels;
  esom::Matrix data;

  Dataset();
  Dataset(esom::Matrix data);
  Dataset(const Dataset& dataset);
  ~Dataset();

  Dataset& operator=(const Dataset&);

  void transpose();
  void copy_columns(const std::vector<unsigned int>& indices, Dataset& out) const;
  void copy_rows(const std::vector<unsigned int>& indices, Dataset& out) const;

  void resize(unsigned int rows, unsigned int cols);
  unsigned int inputs() const {
    return data.rows();
  }
  unsigned int dimension() const {
    return data.columns();
  }
  unsigned int rows() const {
    return data.rows();
  }
  unsigned int columns() const {
    return data.columns();
  }
  bool empty() const {
    return ((rows() == 0) || (columns() == 0));
  }
};

#if 0
class StatDataset : public Dataset {
  double *p_tstat;
 public:
  esom::Matrix data_tstat;
  StatDataset();
  StatDataset(esom::Matrix data);
  StatDataset(const Dataset& dataset);
  StatDataset(const StatDataset& dataset);
  ~StatDataset();

  StatDataset& operator=(const StatDataset&);

  void calc_tstat();
};
#endif

#endif
