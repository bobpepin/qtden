#include <sstream>
#include <algorithm>

#include "datamodels.h"

QVariant AbstractDataModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.row() >= (int)rowCount())
    return QVariant();

  if (role == Qt::DisplayRole) {
    return this->displayData(id(index.row()), index.column());
  }
  if (role == Qt::CheckStateRole) {
    return this->checkState(id(index.row()), index.column());
  }
  return QVariant();
}

void OrderedDataModel::moveRow(unsigned int index, int step) {
  if(((int)index) + step < 0 || index + step >= order.size()) {
    return;
  }
  std::cerr << "moveRow " << index << " -> " << index+step << std::endl;
  emit layoutAboutToBeChanged();
  std::swap(order[index], order[index+step]);
  for(unsigned int i=0; i < order.size(); i++) {
    std::cerr << order[i] << " ";
  }
  std::cerr << std::endl;
  emit layoutChanged();
}

int ReplicateDataModel::rowCount(const QModelIndex &parent) const {
  return metab_data.replicates.columns();
}

int ReplicateDataModel::columnCount(const QModelIndex &parent) const {
  return 4;
}

QVariant ReplicateDataModel::displayData(const unsigned int row, 
					 const unsigned int column) const {
  switch(column) {
  case 0:
    return QString(metab_data.replicates.column_labels.name(row).c_str());
  case 1:
    {
      unsigned int group = labels.group(row);
      unsigned int group_id = labels.group_id(group);
      //      std::cerr << "group[" << row << "] = " << group << std::endl;
      int sample = metab_data.samples.column_labels.index(group_id);
      if(sample != -1) {
	return QString(metab_data.samples.column_labels.name(sample).c_str());
      } else {
	return QString("None");
      }
    }
  case 2: {
    double sum = 0;
    for(unsigned int i=0; i < metab_data.replicates.rows(); i++) {
      sum += metab_data.replicates.data(i, row);
    }
    return sum;
  }
  case 3:
  default:
    return scale_factors[row];
  }
}

QVariant ReplicateDataModel::headerData(int section, 
					Qt::Orientation orientation,
					int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch(section) {
    case 0:
      return QString("Replicate");
    case 1:
      return QString("Sample");
    case 2:
      return QString("Sum");
    case 3:
    default:
      return QString("Multiply by");
    }
  } else {
    return metab_data.replicates.column_labels.id(section);
  }
}

QVariant ReplicateDataModel::checkState(const unsigned int row, 
					const unsigned int column) const {
  if(column != 0) {
    return QVariant();
  }
  return include[order[row]] ? Qt::Checked : Qt::Unchecked;
}

Qt::ItemFlags ReplicateDataModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if(index.column() == 0) {
    f |= Qt::ItemIsUserCheckable;
  }
  if(index.column() == 3) {
    f |= Qt::ItemIsEditable;
  }
  return f;
}

bool ReplicateDataModel::setData(const QModelIndex& index,
				 const QVariant &value,
				 int role) {
  //  std::cerr << "setData " << value.toBool() << std::endl;
  if(index.column() == 0) {
    include[order[index.row()]] = value.toBool();
    emit dataChanged(index, index);
    return true;
  }
  if(index.column() == 3) {
    //    std::cerr << "setData scale = " << value.toDouble() << std::endl;
    double val = value.toDouble();
    if(val != 0) {
      scale_factors[id(index.row())] = val;
    }
    return true;
  }
  return false;
}

int SampleDataModel::rowCount(const QModelIndex &parent) const {
  return labels.size();
}

int SampleDataModel::columnCount(const QModelIndex &parent) const {
  return 3;
}

QVariant SampleDataModel::displayData(unsigned int row, unsigned int column) 
  const
{
  switch(column) {
  case 0:
    return QString(metab_data.samples.column_labels.name(row).c_str());
  case 1:
    {
      unsigned int group = labels.group(row);
      unsigned int group_id = labels.group_id(group);
      //      std::cerr << "group[" << row << "] = " << group << std::endl;
      int sample_group = group_labels.index(group_id);
      if(sample_group != -1) {
	return QString(group_labels.name(sample_group).c_str());
      } else {
	return QString("None");
      }
    }
  case 2:
  default:
    unsigned int count = 0;
    unsigned int id = labels.id(row);
    for(unsigned int i=0; i < metab_data.replicates.column_labels.size(); i++) {
      unsigned int group = metab_data.replicates.column_labels.group(i);
      if(metab_data.replicates.column_labels.group_id(group) == id) {
	count++;
      }
    }
    return count;
  }
}

QVariant SampleDataModel::headerData(int section, 
				     Qt::Orientation orientation,
				     int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch(section) {
    case 0:
      return QString("Sample");
    case 1:
      return QString("Group");
    case 2:
    default:
      return QString("Replicates");
    }
  } else {
    return metab_data.samples.column_labels.id(section);
  }
}

QVariant SampleDataModel::checkState(const unsigned int row, 
				     const unsigned int column) const {
  if(column != 0) {
    return QVariant();
  }
  //  std::cerr << "checkState " << row << " = " << include[order[row]];
  return include[order[row]] ? Qt::Checked : Qt::Unchecked;
}

Qt::ItemFlags SampleDataModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if(index.column() == 0) {
    f |= Qt::ItemIsUserCheckable;
  }
  return f;
}

bool SampleDataModel::setData(const QModelIndex& index,
			      const QVariant &value,
			      int role) {
  //  std::cerr << "setData " << value.toBool() << std::endl;
  if(index.column() == 0) {
    include[order[index.row()]] = value.toBool();
    emit dataChanged(index, index);
    return true;
  }
  return false;
}

QVariant SampleGroupDataModel::displayData(unsigned int row, unsigned int column) 
  const
{
  switch(column) {
  case 0:
    return labels.name(row).c_str();
  case 1:
  default:
    unsigned int count = 0;
    unsigned int id = labels.id(row);
    for(unsigned int i=0; i < metab_data.samples.column_labels.size(); i++) {
      unsigned int group = metab_data.samples.column_labels.group(i);
      if(metab_data.samples.column_labels.group_id(group) == id) {
	count++;
      }
    }
    return count;
  }
}

QVariant SampleGroupDataModel::headerData(int section, 
					Qt::Orientation orientation,
					int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch(section) {
    case 0:
      return QString("Sample Group");
    case 1:
    default:
      return QString("Samples");
    }
  } else {
    return section;
  }
}



int MetaboliteDataModel::rowCount(const QModelIndex &parent) const {
  //  std::cerr << "rowCount = " << metab_data.replicates.rows() << std::endl;
  return metab_data.replicates.rows();
}

int MetaboliteDataModel::columnCount(const QModelIndex &parent) const {
  return 2;
}

QVariant MetaboliteDataModel::displayData(unsigned int row, 
					  unsigned int column) const
{
  switch(column) {
  case 0:
    return QString(metab_data.replicates.row_labels.name(row).c_str());
  case 1:
  default:
    {
      unsigned int hits = 
	metab_data.hits[row];
      return hits;
    }
  }
}

QVariant MetaboliteDataModel::headerData(int section, 
					Qt::Orientation orientation,
					int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch(section) {
    case 0:
      return QString("Feature");
    case 1:
    default:
      return QString("Hits");
    }
  } else {
    return metab_data.replicates.row_labels.id(section);
  }
}

QVariant MetaboliteDataModel::checkState(const unsigned int row, 
				     const unsigned int column) const {
  if(column != 0) {
    return QVariant();
  }
  //  std::cerr << "checkState " << row << " = " << include[order[row]];
  if(include.count(id(row))) {
    return Qt::Checked;
  } else if(exclude.count(id(row))) {
    return Qt::Unchecked;
  } else {
    return Qt::PartiallyChecked;
  }
}

Qt::ItemFlags MetaboliteDataModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  if(index.column() == 0) {
    f |= Qt::ItemIsUserCheckable | Qt::ItemIsTristate;
  }
  return f;
}

bool MetaboliteDataModel::setData(const QModelIndex& index,
				 const QVariant &value,
				 int role) {
  std::cerr << "setData " << value.toInt() << std::endl;
  if(index.column() == 0) {
    unsigned int id = this->id(index.row());
    if(include.count(id)) {
      include.erase(id);
      exclude.insert(id);
    } else if(exclude.count(id)) {
      exclude.erase(id);
    } else {
      include.insert(id);
    }
    emit dataChanged(index, index);
    return true;
  }
  return false;
}
