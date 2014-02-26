#ifndef QTDEN_PARAM_WINDOW_H
#define QTDEN_PARAM_WINDOW_H

#include <functional>
#include <algorithm>

#include <QWidget>
#include <QAbstractListModel>
#include <QTableView>
#include <QTableWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QRadioButton>

#include "metabdata.h"
#include "datamodels.h"

class ParamWindow : public QWidget {
  Q_OBJECT

 private:
  MetabData *input_data;
  ReplicateDataModel *replicates_model;
  QTableView *repl_table;
  SampleDataModel *samples_model;
  QTableView *samp_table;
  SampleGroupDataModel *sample_groups_model;
  QTableView *samp_groups_table;
  Labels samp_group_labels;
  MetaboliteDataModel *metabolites_model;
  QTableView *metab_table;

  QCheckBox *hits_check;
  QSpinBox *hits_spin;
  QCheckBox *anova_check;
  QDoubleSpinBox *anova_spin;
  QCheckBox *anova2_check;
  QDoubleSpinBox *anova2_spin;

  QSpinBox *width_spin;
  QSpinBox *height_spin;
  QSpinBox *epochs_spin;
  QSpinBox *radius_spin;
  QDoubleSpinBox *alpha_spin;
  QRadioButton *dist_corr;
  QRadioButton *dist_eucl;
  QRadioButton *cluster_samples;
  QRadioButton *cluster_replicates;
  QCheckBox *transpose_check;

 public slots:
  void upRepl() {
    moveSelectedRows(repl_table, replicates_model, -1);
  }
  void downRepl() {
    moveSelectedRows(repl_table, replicates_model, +1);
  }
  void upSample() {
    moveSelectedRows(samp_table, samples_model, -1);
  }
  void downSample() {
    moveSelectedRows(samp_table, samples_model, +1);
  }
  void setReplicateGroup() {
    setGroupFromSelection(repl_table, replicates_model, 
			  samp_table, samples_model);
    samples_model->reload();
  }
  void setSampleGroup() {
    setGroupFromSelection(samp_table, samples_model, 
			  samp_groups_table, sample_groups_model);
    sample_groups_model->reload();
  }

  void newSample();
  void newSampleGroup();
  void deleteSample();
  void deleteSampleGroup();

  void smartGroups();
  void run();

  void enableButtons();

 public:
  void moveSelectedRows(QTableView *table, OrderedDataModel *model, 
			int step); 
  void setGroupFromSelection(QTableView *dst_table, AbstractDataModel *dst_model,
			     QTableView *src_table, AbstractDataModel *src_model);
  ParamWindow(QWidget *parent=0);
  void load(MetabData&);
};

#endif
