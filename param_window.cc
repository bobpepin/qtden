#include <sstream>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QProgressDialog>
#include <QButtonGroup>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QTableView>
#include <QRegExp>
#include <QInputDialog>
#include <QLineEdit>
#include <QSplitter>

#include "param_window.h"
#include "metabresultwidget.h"

ParamWindow::ParamWindow(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *layout = new QVBoxLayout();
  QSplitter *splitter = new QSplitter(Qt::Vertical, this);
  this->setLayout(layout);

  QGroupBox *meas_group = new QGroupBox;
  QHBoxLayout *meas_layout = new QHBoxLayout;
  
  meas_group->setLayout(meas_layout);
  
  QGroupBox *replicates = new QGroupBox("Replicates");
  QHBoxLayout *repl_layout = new QHBoxLayout;
  replicates->setLayout(repl_layout);
  this->repl_table = new QTableView();
  this->repl_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  repl_layout->addWidget(this->repl_table, 0, Qt::AlignBottom);

  QWidget *replicate_buttons = new QWidget;
  QVBoxLayout *repl_button_layout = new QVBoxLayout;
  replicate_buttons->setLayout(repl_button_layout);
  QPushButton *set_sample_button = 
    new QPushButton(QIcon(":/icons/merge.png"), "");
  QObject::connect(set_sample_button, SIGNAL(clicked()),
		   SLOT(setReplicateGroup()));
  QPushButton *repl_move_up_button = 
    new QPushButton(QIcon(":/icons/arrow-up.png"), "");
  QObject::connect(repl_move_up_button, SIGNAL(clicked()), SLOT(upRepl()));
  QPushButton *repl_move_down_button = 
    new QPushButton(QIcon(":/icons/arrow-down.png"), "");
  QObject::connect(repl_move_down_button, SIGNAL(clicked()), SLOT(downRepl()));
  repl_button_layout->addWidget(repl_move_up_button);
  repl_button_layout->addWidget(repl_move_down_button);
  repl_button_layout->addWidget(set_sample_button);
  QPushButton *smart_button = new QPushButton(QIcon(":/icons/help-hint.png"), 
					      "");
  QObject::connect(smart_button, SIGNAL(clicked()), SLOT(smartGroups()));
  repl_button_layout->addWidget(smart_button);

  QGroupBox *samples = new QGroupBox("Samples");
  QVBoxLayout *samp_layout = new QVBoxLayout;
  samples->setLayout(samp_layout);
  this->samp_table = new QTableView();
  this->samp_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  QPushButton *new_sample_button = 
    new QPushButton(QIcon(":/icons/list-add.png"), "");
  QObject::connect(new_sample_button, SIGNAL(clicked()), SLOT(newSample()));
  //  QPushButton *delete_sample_button = new QPushButton("Delete");
  //  QObject::connect(delete_sample_button, SIGNAL(clicked()), SLOT(deleteSample()));
  samp_layout->addWidget(this->samp_table);
  //  samp_layout->addWidget(delete_sample_button);

  QWidget *sample_buttons = new QWidget;
  QVBoxLayout *samp_button_layout = new QVBoxLayout;
  sample_buttons->setLayout(samp_button_layout);
  QPushButton *set_sample_group_button =
    new QPushButton(QIcon(":/icons/merge.png"), "");
  QObject::connect(set_sample_group_button, SIGNAL(clicked()),
		   SLOT(setSampleGroup()));
  QPushButton *samp_move_up_button = 
    new QPushButton(QIcon(":/icons/arrow-up.png"), "");
  QObject::connect(samp_move_up_button, SIGNAL(clicked()), 
		   SLOT(upSample()));
  QPushButton *samp_move_down_button =
    new QPushButton(QIcon(":/icons/arrow-down.png"), "");
  QObject::connect(samp_move_down_button, SIGNAL(clicked()), 
		   SLOT(downSample()));
  samp_button_layout->addWidget(samp_move_up_button);
  samp_button_layout->addWidget(samp_move_down_button);
  samp_button_layout->addWidget(set_sample_group_button);
  samp_button_layout->addWidget(new_sample_button);

  QGroupBox *sample_groups = new QGroupBox("Sample Groups");
  QVBoxLayout *samp_groups_layout = new QVBoxLayout;
  sample_groups->setLayout(samp_groups_layout);
  this->samp_groups_table = new QTableView();
  samp_groups_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  samp_groups_table->setSelectionMode(QAbstractItemView::SingleSelection);
  //  QPushButton *delete_sample_group_button = new QPushButton("Delete");
  //  QObject::connect(delete_sample_group_button, SIGNAL(clicked()), SLOT(deleteSampleGroup()));
  samp_groups_layout->addWidget(this->samp_groups_table);
  //  samp_groups_layout->addWidget(new_sample_group_button);
  //  samp_groups_layout->addWidget(delete_sample_group_button);

  QWidget *sample_group_buttons = new QWidget;
  QVBoxLayout *samp_group_button_layout = new QVBoxLayout;
  sample_group_buttons->setLayout(samp_group_button_layout);
  QPushButton *new_sample_group_button = 
    new QPushButton(QIcon(":/icons/list-add.png"), "");
  QObject::connect(new_sample_group_button, SIGNAL(clicked()), SLOT(newSampleGroup()));
  samp_group_button_layout->addWidget(new_sample_group_button);

  meas_layout->addWidget(replicates, 3, Qt::AlignBottom);
  meas_layout->addWidget(replicate_buttons, 0, Qt::AlignBottom);
  meas_layout->addWidget(samples, 3, Qt::AlignBottom);
  meas_layout->addWidget(sample_buttons, 0, Qt::AlignBottom);
  meas_layout->addWidget(sample_groups, 2, Qt::AlignBottom);
  meas_layout->addWidget(sample_group_buttons, 0, Qt::AlignBottom);

  QGroupBox *metab_group = new QGroupBox;
  QHBoxLayout *metab_layout = new QHBoxLayout;
  metab_group->setLayout(metab_layout);

  QGroupBox *metab_table_group = new QGroupBox("Features");
  QHBoxLayout *metab_table_layout = new QHBoxLayout;
  metab_table_group->setLayout(metab_table_layout);
  this->metab_table = new QTableView();
  this->metab_table->setSelectionBehavior(QAbstractItemView::SelectRows);
  metab_table_layout->addWidget(this->metab_table);

  QGroupBox *metab_filters = new QGroupBox("Feature Filters");
  QFormLayout *filters_layout = new QFormLayout;
  metab_filters->setLayout(filters_layout);
  hits_check = new QCheckBox("Minimum Hits");
  hits_check->setChecked(true);
  hits_spin = new QSpinBox;
  hits_spin->setRange(0, 1e6);
  filters_layout->addRow(hits_check, hits_spin);
  anova_check = new QCheckBox("ANOVA replicates/samples");
  anova_check->setChecked(true);
  anova_spin = new QDoubleSpinBox();
  anova_spin->setPrefix("p < ");
  anova_spin->setRange(0.0, 1.0);
  anova_spin->setDecimals(5);
  anova_spin->setSingleStep(0.01);
  anova_spin->setValue(0.05);
  filters_layout->addRow(anova_check, anova_spin);

  anova2_check = new QCheckBox("ANOVA samples/sample groups");
  anova2_check->setChecked(true);
  anova2_spin = new QDoubleSpinBox();
  anova2_spin->setPrefix("p < ");
  anova2_spin->setRange(0.0, 1.0);
  anova2_spin->setSingleStep(0.01);
  anova2_spin->setDecimals(5);
  anova2_spin->setValue(0.05);
  filters_layout->addRow(anova2_check, anova2_spin);

  QGroupBox *cluster_settings = new QGroupBox("Cluster Settings");
  QFormLayout *cluster_settings_layout = new QFormLayout;
  cluster_settings->setLayout(cluster_settings_layout);
  QWidget *distance_group = new QWidget;
  QVBoxLayout *distance_layout = new QVBoxLayout;
  distance_group->setLayout(distance_layout);
  dist_corr = new QRadioButton("Correlation");
  dist_corr->setChecked(true);
  dist_eucl = new QRadioButton("Euclidean");
  //  distance_button_group->addButton(dist_corr);
  //  distance_button_group->addButton(dist_eucl);
  distance_layout->addWidget(dist_corr);
  distance_layout->addWidget(dist_eucl);

  QWidget *cluster_group = new QWidget;
  QVBoxLayout *cluster_layout = new QVBoxLayout;
  cluster_group->setLayout(cluster_layout);
  cluster_replicates = new QRadioButton("Replicates");
  cluster_replicates->setChecked(true);
  cluster_samples = new QRadioButton("Samples");
  cluster_layout->addWidget(cluster_replicates);
  cluster_layout->addWidget(cluster_samples);

  cluster_settings_layout->addRow("Distance function", distance_group);
  cluster_settings_layout->addRow("Cluster by", cluster_group);
  transpose_check = new QCheckBox("Transpose");
  cluster_settings_layout->addRow("Transpose", transpose_check);

  QWidget *settings = new QWidget();
  QHBoxLayout *settings_layout = new QHBoxLayout;
  settings->setLayout(settings_layout);
  QGroupBox *esom_group = new QGroupBox("ESOM Parameters");
  QFormLayout *esom_layout = new QFormLayout;
  esom_group->setLayout(esom_layout);
  width_spin = new QSpinBox();
  width_spin->setRange(0, 1e6);
  width_spin->setValue(150);
  height_spin = new QSpinBox();
  height_spin->setRange(0, 1e6);
  height_spin->setValue(100);
  epochs_spin = new QSpinBox();
  epochs_spin->setRange(0, 1e6);
  epochs_spin->setValue(5);
  //  QButtonGroup *distance_button_group = new QButtonGroup;
  radius_spin = new QSpinBox();
  radius_spin->setRange(1, 1e6);
  radius_spin->setValue(50);
  alpha_spin = new QDoubleSpinBox();
  alpha_spin->setRange(0.1, 1.0);
  alpha_spin->setSingleStep(0.1);
  alpha_spin->setValue(0.5);

  esom_layout->addRow("Width", width_spin);
  esom_layout->addRow("Height", height_spin);
  esom_layout->addRow("Epochs", epochs_spin);
  esom_layout->addRow("Initial radius", radius_spin);
  esom_layout->addRow("Initial alpha", alpha_spin);

  settings_layout->addWidget(metab_filters);
  settings_layout->addWidget(cluster_settings);
  settings_layout->addWidget(esom_group);

  metab_layout->addWidget(metab_table_group);

  QPushButton *run_button = new QPushButton("Run");
  QObject::connect(run_button, SIGNAL(clicked()), SLOT(run()));

  splitter->addWidget(meas_group);
  splitter->addWidget(metab_group);
  splitter->addWidget(settings);
  layout->addWidget(splitter);
  layout->addWidget(run_button);

}

void ParamWindow::moveSelectedRows(QTableView *table, OrderedDataModel *model, 
				  int step) {
  QItemSelectionModel *selectionModel = table->selectionModel();
  QModelIndexList indexes = selectionModel->selectedIndexes();
  QModelIndex index;
  std::vector<unsigned int> rows;
    
  foreach(index, indexes) {
    rows.push_back(index.row());
  }

  if(step > 0) {
    std::sort(rows.begin(), rows.end(), std::greater<unsigned int>());
  } else {
    std::sort(rows.begin(), rows.end());
  }
  std::vector<unsigned int>::iterator it;
  it = std::unique(rows.begin(), rows.end());
  rows.resize(it - rows.begin());

  if(rows.size() == 0 || ((int)rows[0])+step < 0 || 
     ((int)rows[0]) + step >= model->rowCount()) {
    return;
  }
  
  selectionModel->select(QModelIndex(), QItemSelectionModel::Clear);
  for(unsigned int i=0; i < rows.size(); i++) {
    model->moveRow(rows[i], step);
    QModelIndex idx = replicates_model->index(rows[i]+step, 0, QModelIndex());
    selectionModel->select(idx, QItemSelectionModel::Select | 
			   QItemSelectionModel::Rows);
  }
}

void ParamWindow::setGroupFromSelection(QTableView *dst_table, 
					AbstractDataModel *dst_model,
					QTableView *src_table, 
					AbstractDataModel *src_model) {
  std::cerr << "setGroupFromSelection " << std::endl;
  QItemSelectionModel *srcSelectionModel = src_table->selectionModel();
  QModelIndexList src_indexes = srcSelectionModel->selectedIndexes();
  std::cerr << "setGroupFromSelection " << src_indexes.size() << std::endl;
  if(src_indexes.size() == 0) {
    return;
  }
  int src_row = src_indexes[1].row();
  QModelIndex src_index;
  foreach(src_index, src_indexes) {
    if(src_index.row() != src_row) {
      return;
    }
  }
  unsigned int src_id = src_model->id(src_row);

  QItemSelectionModel *selectionModel = dst_table->selectionModel();
  QModelIndexList indexes = selectionModel->selectedIndexes();
  QModelIndex index;
    
  foreach(index, indexes) {
    dst_model->setGroup(index.row(), src_id);
  }
}

void ParamWindow::load(MetabData& data) {
  replicates_model = new ReplicateDataModel(data);
  repl_table->setModel(replicates_model);
  samples_model = new SampleDataModel(data, samp_group_labels);
  samp_table->setModel(samples_model);
  sample_groups_model = new SampleGroupDataModel(data, samp_group_labels);
  samp_groups_table->setModel(sample_groups_model);
  metabolites_model = new MetaboliteDataModel(data);
  metab_table->setModel(metabolites_model);
  this->input_data = &data;
  QObject::connect(samples_model, SIGNAL(layoutChanged()), 
		   SLOT(enableButtons()));
  QObject::connect(sample_groups_model, SIGNAL(layoutChanged()),
		   SLOT(enableButtons()));
  repl_table->resizeColumnsToContents();
  samp_table->resizeColumnsToContents();
  metab_table->resizeColumnsToContents();
  enableButtons();
}

void groupsFromRegex(const char* regex, const Dataset& dataset, 
		     std::map<unsigned int, unsigned int>& group_map,
		     std::map<unsigned int, std::string>& name_map) {
  std::map<std::string, unsigned int> group_ids;
  unsigned int next_gid = 0;
  QRegExp re(regex);
  re.setPatternSyntax(QRegExp::RegExp2);
  for(unsigned int i=0; i < dataset.columns(); i++) {
    std::string name = dataset.column_labels.name(i);
    if(re.indexIn(name.c_str()) != -1 && re.captureCount() > 0) {
      std::string group_name = re.cap(1).toStdString();
      unsigned int gid;
      if(!group_ids.count(group_name)) {
	gid = next_gid++;
	group_ids[group_name] = gid;
      } else {
	gid = group_ids[group_name];
      }
      group_map[i] = gid;
    }
  }
  std::map<std::string, unsigned int>::const_iterator it;
  for(it = group_ids.begin(); it != group_ids.end(); it++) {
    std::pair<std::string, unsigned int> pair = *it;
    name_map[pair.second] = pair.first;
  }
}

void ParamWindow::newSample() {
  bool ok;
  QString name = QInputDialog::getText(this, "New sample", "Sample name",
					QLineEdit::Normal, "", &ok);
  if(!ok || name.isEmpty()) return;
  Labels& labels = input_data->samples.column_labels;
  labels.setName(labels.size(), name.toStdString());
  samples_model->reload();
}

void ParamWindow::deleteSample() {}
void ParamWindow::newSampleGroup() {
  bool ok;
  QString name = QInputDialog::getText(this, "New sample", "Sample name",
					QLineEdit::Normal, "", &ok);
  if(!ok || name.isEmpty()) return;
  Labels& labels = samp_group_labels;
  labels.setName(labels.size(), name.toStdString());
  sample_groups_model->reload();
}


void ParamWindow::deleteSampleGroup() {}

void ParamWindow::smartGroups() {
  bool ok;
  const char *msg = "Regular expression to extract base name for samples";
  QString regex = QInputDialog::getText(this, "Extract sample names", msg,
					QLineEdit::Normal, "(.*)_", &ok);
  if(!ok || regex.isEmpty()) return;
  std::map<unsigned int, std::string> group_names;
  std::map<unsigned int, unsigned int> group_map;
  groupsFromRegex(regex.toStdString().c_str(), this->input_data->replicates, group_map, group_names);
  std::map<unsigned int, std::string>::const_iterator it;
  for(it = group_names.begin(); it != group_names.end(); it++) {
    std::pair<unsigned int, std::string> pair = *it;
    //    std::cerr << "group " << pair.first << " " << pair.second << std::endl;
  }
  input_data->samples.column_labels.resize(0);
  input_data->samples.column_labels.resize(group_names.size());
  input_data->samples.column_labels.setIdNames(group_names);
  input_data->replicates.column_labels.setGroups(group_map);
  replicates_model->reload();
  samples_model->reload();
  samp_table->resizeColumnsToContents();
  repl_table->resizeColumnsToContents();
  cluster_samples->setChecked(true);
  hits_spin->setValue(input_data->replicates.columns() / 2);
}

void ParamWindow::enableButtons() {
  if(samples_model) {
    anova_check->setEnabled(samples_model->rowCount() > 1);
  }
  if(sample_groups_model) {
    anova2_check->setEnabled(sample_groups_model->rowCount() > 1);
    cluster_samples->setEnabled(samples_model->rowCount() > 0);
  }
}

void ParamWindow::run() {
  MetabParams params;
  params.replicate_order = replicates_model->getOrder();
  params.replicate_factors = replicates_model->scale_factors;
  params.sample_order = samples_model->getOrder();
  params.metabolites_include.resize(metabolites_model->include.size());
  copy(metabolites_model->include.begin(), metabolites_model->include.end(), 
       params.metabolites_include.begin());
  params.metabolites_exclude.resize(metabolites_model->exclude.size());
  copy(metabolites_model->exclude.begin(), metabolites_model->exclude.end(), 
       params.metabolites_exclude.begin());
  if(anova_check->isEnabled() && anova_check->isChecked()) {
    params.anova_replicate_threshold = anova_spin->value();
  } else {
    params.anova_replicate_threshold = 0;
  }
  if(anova2_check->isEnabled() && anova2_check->isChecked()) {
    params.anova_sample_threshold = anova2_spin->value();
  } else {
    params.anova_sample_threshold = 0;
  }
  if(hits_check->isChecked()) {
    params.hits_threshold = hits_spin->value();
  } else {
    params.hits_threshold = 0;
  }
  params.esom_width = width_spin->value();
  params.esom_height = height_spin->value();
  params.esom_epochs = epochs_spin->value();
  params.esom_radius = radius_spin->value();
  params.esom_alpha = alpha_spin->value();
  if(dist_corr->isChecked()) {
    params.esom_distance = MetabParams::Correlation;
  } else {
    params.esom_distance = MetabParams::Euclidean;
  }
  if(cluster_samples->isEnabled() && cluster_samples->isChecked()) {
    params.esom_data = MetabParams::Samples;
  } else {
    params.esom_data = MetabParams::Replicates;
  }
#if 1
  if(transpose_check->isChecked()) {
    params.esom_transpose = true;
  } else {
    params.esom_transpose = false;
  }
#else
  params.esom_transpose = false;
#endif
  MetabAnalysis *analysis = new MetabAnalysis(*input_data, params);
  if(analysis->metab_data.rows() == 0) {
    QMessageBox::critical(this, "Empty dataset", "No features passed the filters, check the \"Feature Filters\" settings.");
    return;
  }
  analysis->beginRun();
  std::stringstream ss;
  ss << "Training SOM (" << analysis->metab_data.rows() << " rows)";
  QProgressDialog progress(ss.str().c_str(), "Cancel", 0, params.esom_epochs, 
			   this);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(0);
  progress.show();
  for(unsigned int i=0; i < params.esom_epochs; i++) {
    progress.setValue(i);
    if(progress.wasCanceled()) {
      break;
    }
    analysis->runEpoch();
  }
  if(progress.wasCanceled()) {
    progress.reset();
    return;
  }
  analysis->endRun();
  MetabResultWidget* window = new MetabResultWidget(analysis);
  window->show();
}
