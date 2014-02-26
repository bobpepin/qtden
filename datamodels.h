#ifndef QTDEN_DATAMODELS_H
#define QTDEN_DATAMODELS_H

#include <QAbstractListModel>

#include "dataset.h"
#include "metabdata.h"

class AbstractDataModel : public QAbstractTableModel {
  Q_OBJECT

 protected:
  MetabData& metab_data; 
  Labels& labels;
 public:

 AbstractDataModel(MetabData& metab_data, Labels& labels,
		   QObject *parent = 0)
   : QAbstractTableModel(parent), 
    metab_data(metab_data), labels(labels) {
  }

  virtual unsigned int id(unsigned int pos) const {
    return pos;
  }
  void setGroup(unsigned int pos, unsigned int group) {
    std::cerr << "setGroup " << pos << " " << group << std::endl;
    labels.setGroup(id(pos), group);
    emit dataChanged(index(pos, 0, QModelIndex()), 
		     index(pos, this->columnCount()-1, QModelIndex()));
  }
  virtual void reload() {
    //    std::cerr << "reload: " << rowCount() << std::endl;
    //    unsigned int oldsize = order.size();
    emit layoutChanged();
    emit dataChanged(index(0, 0, QModelIndex()), 
		     index(rowCount()-1, columnCount()-1, QModelIndex()));
  }
  QVariant data(const QModelIndex &index, int role) const;
  virtual QVariant displayData(const unsigned int row, const unsigned int column) 
    const = 0;
  virtual QVariant checkState(const unsigned int, const unsigned int)
    const { return QVariant(); }
};

class OrderedDataModel : public AbstractDataModel {
  Q_OBJECT
 protected:
  std::vector<unsigned int> order;
  std::vector<bool> include;
  
 public:
  OrderedDataModel(MetabData& metab_data, Labels& labels, QObject *parent = 0) :
  AbstractDataModel(metab_data, labels, parent), order(0), 
    include(labels.size(), 1) {
    order.resize(labels.size());
    for(unsigned int i=0; i < order.size(); i++) {
      order[i] = i;
    }
  }

  void reload() {
    order.resize(labels.size());
    include.resize(labels.size());
    for(unsigned int i=0; i < order.size(); i++) {
      order[i] = i;
      include[i] = 1;
    }
    AbstractDataModel::reload();
  }

  unsigned int id(unsigned int pos) const {
    return order[pos];
  }

  virtual std::vector<unsigned int> getOrder() {
    std::vector<unsigned int> r;
    for(unsigned int i=0; i < order.size(); i++) {
      if(include[order[i]]) {
	r.push_back(order[i]);
      }
    }
    return r;
  }

 public slots:
  void moveRow(unsigned int index, int step);
  
};

class ReplicateDataModel : public OrderedDataModel {
  Q_OBJECT
 public:
  std::vector<double> scale_factors;
 ReplicateDataModel(MetabData& metab_data, QObject *parent = 0)
   : OrderedDataModel(metab_data, metab_data.replicates.column_labels,
		      parent), scale_factors(labels.size(), 1.0) {
  }
  void reload() {
    scale_factors.resize(labels.size());
    std::fill(scale_factors.begin(), scale_factors.end(), 1.0);
    OrderedDataModel::reload();
  };
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant displayData(const unsigned int row, const unsigned int column) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;
  QVariant checkState(const unsigned int, const unsigned int) const;
  bool setData(const QModelIndex& index, const QVariant& value, int role);
  Qt::ItemFlags flags(const QModelIndex&) const;
};

class SampleDataModel : public OrderedDataModel {
  Q_OBJECT

 private:
  const Labels& group_labels;
 public:
  SampleDataModel(MetabData& metab_data, const Labels& group_labels, 
		   QObject *parent = 0)
    : OrderedDataModel(metab_data, metab_data.samples.column_labels, parent),
      group_labels(group_labels) {}

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant displayData(const unsigned int row, const unsigned int column) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;
  QVariant checkState(const unsigned int, const unsigned int) const;
  bool setData(const QModelIndex& index, const QVariant& value, int role);
  Qt::ItemFlags flags(const QModelIndex&) const;
};

class SampleGroupDataModel : public AbstractDataModel {
  Q_OBJECT
 public:
  SampleGroupDataModel(MetabData& metab_data, Labels& labels,
		       QObject *parent = 0)
    : AbstractDataModel(metab_data, labels, parent) {}
  int rowCount(const QModelIndex & = QModelIndex()) const {
    return labels.size();
  }
  int columnCount(const QModelIndex & = QModelIndex()) const {
    return 2;
  }
  QVariant displayData(const unsigned int row, const unsigned int column) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;
};

class MetaboliteDataModel : public AbstractDataModel {
  Q_OBJECT

 public:
  std::set<unsigned int> exclude;
  std::set<unsigned int> include;

 MetaboliteDataModel(MetabData& metab_data, QObject *parent = 0)
   : AbstractDataModel(metab_data, metab_data.replicates.row_labels, parent) {}
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant displayData(const unsigned int row, const unsigned int column) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;
  QVariant checkState(const unsigned int, const unsigned int) const;
  bool setData(const QModelIndex& index, const QVariant& value, int role);
  Qt::ItemFlags flags(const QModelIndex&) const;
};

#endif
