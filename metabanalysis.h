#ifndef QTDEN_METAB_ANALYSIS
#define QTDEN_METAB_ANALYSIS

#include <vector>

#include <QMainWindow>

#include "param_window.h"
#include "metabdata.h"

struct MetabParams {
  std::vector<unsigned int> replicate_order;
  std::vector<double> replicate_factors;
  std::vector<unsigned int> sample_order;
  std::vector<unsigned int> metabolites_include;
  std::vector<unsigned int> metabolites_exclude;
  double anova_replicate_threshold;
  double anova_sample_threshold;
  unsigned int hits_threshold;
  unsigned int esom_width;
  unsigned int esom_height;
  unsigned int esom_epochs;
  unsigned int esom_radius;
  double esom_alpha;
  enum { Correlation, Euclidean } esom_distance;
  enum { Replicates, Samples } esom_data;
  bool esom_transpose;
};

class MetabAnalysis {
 private:
  esom::BestMatchFunction *bestmatch;
  esom::OnlineSOM *som;
  Dataset *subset;
  Dataset *norm_subset;
  esom::DistanceFunction *distance;
  esom::CoolingFunction *cooling;
  esom::CoolingFunction *radiusCooling;
  esom::NeighbourhoodFunction *neighbourhood;
 public:
  MetabParams params;
  MetabData metab_data;
  esom::Grid *grid;
  esom::LabelTree tree;
  esom::UMatrix *umatrix;
  std::vector<int> bestmatches;
  
  MetabAnalysis(const MetabData& input_data,
		const MetabParams& params);
  ~MetabAnalysis();
  void beginRun();
  void runEpoch();
  void endRun();
  void run();
};

#endif
