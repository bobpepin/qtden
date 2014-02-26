#include "resulttablewidget.h"

#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QVBoxLayout>
#include <QPrinter>
#include <QPrintDialog>
#include <QFileDialog>
#include <QMessageBox>

ResultTableModel::ResultTableModel(const Dataset& dataset, 
				   std::vector<unsigned int>& tree_positions,
				   QObject *parent)
    : QAbstractTableModel(parent), dataset(dataset), 
      tree_positions(tree_positions) { 
  order.resize(rowCount());
  for(unsigned int i=0; i < order.size(); i++) {
    order[i] = i;
  }
}


int ResultTableModel::rowCount(const QModelIndex& parent) const {
  return dataset.rows();
}

int ResultTableModel::columnCount(const QModelIndex& parent) const {
  return dataset.columns() + 3;
}

QVariant ResultTableModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid())
    return QVariant();

  if (index.row() >= (int)rowCount())
    return QVariant();

  unsigned int row = order[index.row()];

  if (role == Qt::DisplayRole) {
    if(index.column() >= 3) {
      return dataset.data(row, index.column() - 3);
    } else if(index.column() == 0) {
      return QString(dataset.row_labels.name(row).c_str());
    } else if(index.column() == 1) {
      return dataset.row_labels.id(row);
    } else {
      return tree_positions[row];
    }
  }

  return QVariant();
}

QVariant ResultTableModel::headerData(int section, 
				      Qt::Orientation orientation,
				      int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch(section) {
    case 0:
      return QString("Feature");
    case 1:
      return QString("Id");
    case 2:
      return QString("Ordinate");
    default:
      return QString(dataset.column_labels.name(section - 3).c_str());
    }
  } else {
    return section;
  }
}

void ResultTableModel::sort(int column, Qt::SortOrder sortOrder) {
  bool reverse = sortOrder == Qt::AscendingOrder;
  if(column == 0) {
    NameSorter sorter(dataset, reverse);
    std::sort(order.begin(), order.end(), sorter);
  } else if(column == 1) {
    IdSorter sorter(dataset, reverse);
    std::sort(order.begin(), order.end(), sorter);
  } else if(column == 2) {
    PositionSorter sorter(dataset, tree_positions, reverse);
    std::sort(order.begin(), order.end(), sorter);
  } else {
    DataSorter sorter(dataset, column - 3, reverse);
    std::sort(order.begin(), order.end(), sorter);
  }
  emit layoutChanged();
}

bool ResultTableModel::exportCsv(const char *fname) {
  std::ofstream ofs(fname);
  if(!ofs.good()) {
    return false;
  }
  ofs << "Feature\tId\tOrdinate";
  for(unsigned int i=0; i < dataset.column_labels.size(); i++) {
    ofs << '\t' << dataset.column_labels.name(i);
  }
  ofs << '\n';
  for(unsigned int i=0; i < dataset.rows(); i++) {
    ofs << dataset.row_labels.name(i) << '\t' << dataset.row_labels.id(i)
	<< '\t' << tree_positions[i];
    for(unsigned int j=0; j < dataset.columns(); j++) {
      ofs << '\t' << dataset.data(i, j);
    }
    ofs << '\n';
  }
  return ofs.good();
}

ResultTableWidget::ResultTableWidget(const MetabData& metab_data, 
				     const esom::LabelTree& tree,
				     QWidget *parent)
  : QWidget(parent), metab_data(metab_data) {
  table = new QTableView();
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setSelectionMode(QAbstractItemView::SingleSelection);
  table->setSortingEnabled(true);

  tree_positions.resize(tree.leafCount()+1);
  buildTreePositions(tree, tree.top(), 0);

  replicates_model = 
    new ResultTableModel(metab_data.replicates, tree_positions);
  samples_model = new ResultTableModel(metab_data.samples, tree_positions);

  QToolBar *toolbar = new QToolBar;
  QActionGroup *displayGroup = new QActionGroup(this);
  QAction *replicatesAct = new QAction("Replicates", displayGroup);
  replicatesAct->setCheckable(true);
  QAction *samplesAct = new QAction("Samples", displayGroup);
  samplesAct->setCheckable(true);
   if(!metab_data.samples.empty()) {
    samplesAct->setChecked(true);
    showSamples();
  } else {
    replicatesAct->setChecked(true);
    samplesAct->setEnabled(false);
    showReplicates();
  }
  toolbar->addAction(replicatesAct);
  toolbar->addAction(samplesAct);

  QObject::connect(replicatesAct, SIGNAL(triggered()),
		   SLOT(showReplicates()));
  QObject::connect(samplesAct, SIGNAL(triggered()), SLOT(showSamples()));

  QAction *exportAct = new QAction("Export", toolbar);
  toolbar->addAction(exportAct);
  QObject::connect(exportAct, SIGNAL(triggered()), SLOT(exportData()));

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(toolbar);
  layout->addWidget(table);
  this->setLayout(layout);
}

int ResultTableWidget::buildTreePositions(const esom::LabelTree& tree, int node, int pos) {
  int size = tree.leafCount();
  int tsize = tree_positions.size();
  if(tree.isLeaf(node)) {
    assert(tree_positions.size() > node);
    tree_positions[node] = tree.leafCount() - pos;
    return 1;
  }
  
  int right_count = buildTreePositions(tree, tree.right(node), pos);
  int left_count = buildTreePositions(tree, tree.left(node), pos+right_count);
  return left_count + right_count;
}

void ResultTableWidget::showReplicates() {
  current_model = replicates_model;
  table->setModel(replicates_model);
  table->resizeColumnsToContents();
  QItemSelectionModel *selmodel = table->selectionModel();
  QObject::connect(selmodel, 
		   SIGNAL(currentRowChanged(const QModelIndex&, 
					    const QModelIndex&)), 
		   SLOT(currentChanged(const QModelIndex&,
				       const QModelIndex&)));
}

void ResultTableWidget::showSamples() {
  current_model = samples_model;
  table->setModel(samples_model);
  table->resizeColumnsToContents();
  QItemSelectionModel *selmodel = table->selectionModel();
  QObject::connect(selmodel, 
		   SIGNAL(currentRowChanged(const QModelIndex&, 
					    const QModelIndex&)), 
		   SLOT(currentChanged(const QModelIndex&,
				       const QModelIndex&)));
}

void ResultTableWidget::currentChanged(const QModelIndex& current, 
				       const QModelIndex& previous) {
  //  std::cerr << "currentChanged" << std::endl;
  if(current_model) {
    if(current_model == samples_model) {
      emit selectSample(current_model->rowId(current.row()));
    } else {
      emit selectReplicate(current_model->rowId(current.row()));
    }
  }
}

void ResultTableWidget::exportData() {
  QString filename = QFileDialog::getSaveFileName(this, "Export table");
  if(current_model->exportCsv(filename.toStdString().c_str())) {
    QMessageBox::information(this, "Export successful", 
			     "The data has been exported successfully.");
  } else {
    QMessageBox::critical(this, "Export failed", 
			  "There was an error during the data export.");
  }
}
