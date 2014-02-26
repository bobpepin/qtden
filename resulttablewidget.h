#ifndef QTDEN_RESULTTABLE_WIDGET_H
#define QTDEN_RESULTTABLE_WIDGET_H

#include <QWidget>
#include <esom.h>

#include <QAbstractTableModel>
#include <QTableView>

#include "metabdata.h"
#include "heatmapplot.h"

class ResultTableModel : public QAbstractTableModel {
  Q_OBJECT

 protected:
  const Dataset& dataset;
  const std::vector<unsigned int> tree_positions;
  std::vector<unsigned int> order;
  
  struct DataSorter {
    const Dataset& dataset;
    const unsigned int column;
    const bool reverse;
    DataSorter(const Dataset& dataset, const unsigned int column, 
	       const bool reverse)
      : dataset(dataset), column(column), reverse(reverse) {}
    bool operator() (unsigned int a, unsigned int b) {
      if(reverse) {
       return (dataset.data(a, column) > dataset.data(b, column));
      } else {
	return (dataset.data(a, column) < dataset.data(b, column));
      }
    }
  };

  struct IdSorter {
    const Dataset& dataset;
    const bool reverse;
  IdSorter(const Dataset& dataset, const bool reverse)
    : dataset(dataset), reverse(reverse) {}
    bool operator() (unsigned int a, unsigned int b) {
      if(reverse) {
       return (dataset.row_labels.id(a) > dataset.row_labels.id(b));
      } else {
       return (dataset.row_labels.id(a) < dataset.row_labels.id(b));
      }
    }
  };

  struct NameSorter {
    const Dataset& dataset;
    const bool reverse;
  NameSorter(const Dataset& dataset, const bool reverse)
    : dataset(dataset), reverse(reverse) {}
    bool operator() (unsigned int a, unsigned int b) {
      if(reverse) {
       return (dataset.row_labels.name(a) > dataset.row_labels.name(b));
      } else {
       return (dataset.row_labels.name(a) < dataset.row_labels.name(b));
      }
    }
  };

  struct PositionSorter {
    const Dataset& dataset;
    const std::vector<unsigned int>& pos;
    const bool reverse;
  PositionSorter(const Dataset& dataset, const std::vector<unsigned int>& pos, 
		 const bool reverse)
  : dataset(dataset), pos(pos), reverse(reverse) {}
    bool operator() (unsigned int a, unsigned int b) {
      if(reverse) {
       return (pos[a] > pos[b]);
      } else {
	return (pos[a] < pos[b]);
      }
    }
  };


 public:
  ResultTableModel(const Dataset& dataset, std::vector<unsigned int>& tree_positions,
		   QObject *parent = 0);
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const;
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
  unsigned int rowId(unsigned int row) {
    return order[row];
  }
  bool exportCsv(const char*);
};

class ResultTableWidget : public QWidget {
  Q_OBJECT
 private:
  const MetabData& metab_data;
  QTableView *table;
  ResultTableModel *replicates_model;
  ResultTableModel *samples_model;
  ResultTableModel *current_model;
  std::vector<unsigned int> tree_positions;
  int buildTreePositions(const esom::LabelTree& tree, int node, int pos);
 public:
  ResultTableWidget(const MetabData& metab_data, const esom::LabelTree&,
		    QWidget *parent=0);
 public slots:
  void exportData();
  void showReplicates();
  void showSamples();

 protected slots:
  virtual void currentChanged(const QModelIndex& current, 
		      const QModelIndex& previous);
 signals:
  void selectSample(int);
  void selectReplicate(int);
  void legend(QString);
};

#endif
