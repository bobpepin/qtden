#include "metabanalysis.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <cmath>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif


MetabAnalysis::MetabAnalysis(const MetabData& input_data,
			     const MetabParams& params) : params(params) {

  MetabData tmp_data;
  const Labels& rep_labels = input_data.replicates.column_labels;
  std::vector<unsigned int> rep_indices(params.replicate_order.size());
  for(unsigned int i=0; i < rep_indices.size(); i++) {
    rep_indices[i] = rep_labels.index(params.replicate_order[i]);
  }
  input_data.replicates.copy_columns(rep_indices, tmp_data.replicates);

  for(unsigned int j=0; j < tmp_data.replicates.columns(); j++) {
    unsigned int id = tmp_data.replicates.column_labels.id(j);
    double factor = params.replicate_factors[id];
    for(unsigned int i=0; i < tmp_data.replicates.rows(); i++) {
      tmp_data.replicates.data(i, j) *= factor;
    }
  }

  const Labels& sam_labels = input_data.samples.column_labels;
  std::vector<unsigned int> sam_indices(params.sample_order.size());
  for(unsigned int i=0; i < sam_indices.size(); i++) {
    sam_indices[i] = sam_labels.index(params.sample_order[i]);
    //    std::cerr << "sam_indices[" << i << "] = " << sam_indices[i] << std::endl;
  }
  input_data.samples.copy_columns(sam_indices, tmp_data.samples);

  //  std::cerr << "Replicates: " << tmp_data.replicates.columns()
  //	    << " Samples: " << tmp_data.samples.columns() << std::endl;

  tmp_data.hits = input_data.hits;

  std::vector<unsigned int> anova_replicate_rows;
  std::vector<unsigned int> anova_sample_rows;
  std::vector<unsigned int> hits_rows;
  std::vector<unsigned int> filtered_rows(input_data.rows());
  std::vector<unsigned int>::iterator it;
  for(unsigned int i=0; i < filtered_rows.size(); i++) {
    filtered_rows[i] = i;
  }

  if(params.hits_threshold) {
    hits_rows = hits_test(tmp_data, params.hits_threshold);
    it = set_intersection(hits_rows.begin(), hits_rows.end(),
			  filtered_rows.begin(), filtered_rows.end(),
			  filtered_rows.begin());
    filtered_rows.resize(it - filtered_rows.begin());
  }

  if(params.anova_replicate_threshold) {
    anova_replicate_rows = anova_test(tmp_data.replicates, 
				      params.anova_replicate_threshold);
    it = set_intersection(anova_replicate_rows.begin(), 
			  anova_replicate_rows.end(),
			  filtered_rows.begin(), filtered_rows.end(),
			  filtered_rows.begin());
    filtered_rows.resize(it - filtered_rows.begin());
  }

  if(params.anova_sample_threshold) {
    tmp_data.calc_samples();
    anova_sample_rows = anova_test(tmp_data.samples, 
				   params.anova_sample_threshold);
    it = set_intersection(anova_sample_rows.begin(), 
			  anova_sample_rows.end(),
			  filtered_rows.begin(), filtered_rows.end(),
			  filtered_rows.begin());
    filtered_rows.resize(it - filtered_rows.begin());
  }

  {
    std::vector<unsigned int> tmp_rows(filtered_rows.size());

    it = set_difference(filtered_rows.begin(), filtered_rows.end(),
			params.metabolites_exclude.begin(), 
			params.metabolites_exclude.end(),
			tmp_rows.begin());
    filtered_rows.resize((it - tmp_rows.begin()) 
			 + params.metabolites_include.size());
    it = set_union(tmp_rows.begin(), it,
		   params.metabolites_include.begin(),
		   params.metabolites_include.end(),
		   filtered_rows.begin());
    filtered_rows.resize(it - filtered_rows.begin());
  }

  std::cerr << "ANOVA Samples: " << anova_sample_rows.size() 
	    << " ANOVA Replicates: " << anova_replicate_rows.size()
	    << " Hits: " << hits_rows.size() << std::endl;

  tmp_data.copy_rows(filtered_rows, metab_data);

  std::cerr << "Metabolites after filters: " 
	    << metab_data.rows() << std::endl;
  //  std::cerr << "Samples: " << metab_data.samples.column_labels.size() << std::endl;
}

MetabAnalysis::~MetabAnalysis() {
  if(grid)
    delete grid;
  if(umatrix)
    delete umatrix;
}

void MetabAnalysis::beginRun() {
  metab_data.calc_all();

  if(params.esom_transpose) {
    metab_data.transpose();
  }

  //  std::cerr << "Samples 2: " << metab_data.samples.column_labels.size() << std::endl;
  //  Dataset& subset = metab_data.samples;
  //  Dataset& norm_subset = metab_data.samples_tstat;

  if(params.esom_data == MetabParams::Replicates) {
    subset = &metab_data.replicates;
    norm_subset = &metab_data.replicates_tstat;
  } else {
    subset = &metab_data.samples;
    norm_subset = &metab_data.samples_tstat;
  }

  //  std::cerr << "Replicates: " << metab_data.replicates.data.rows() << std::endl;

  grid = new esom::ToroidGrid(params.esom_width, params.esom_height, 
			      subset->dimension());
  const int epochs = params.esom_epochs;
  //  std::cerr << "epochs: " << epochs << std::endl;
  if(params.esom_distance == MetabParams::Correlation) {
    distance = new esom::distance::Correlation;
  } else {
    distance = new esom::distance::Euclidean;
  }
  bestmatch = new esom::bestmatch::Linear (*distance);
  neighbourhood = new esom::neighbourhood::Gauss;
  cooling = new esom::cooling::Linear(params.esom_alpha, .1, epochs);
  radiusCooling = new esom::cooling::Linear(params.esom_radius, 1, epochs);
  som = new esom::OnlineSOM(*grid, *bestmatch, *neighbourhood, *cooling, 
			    *radiusCooling);

  srand(time(NULL));
  // srand(1);
  som->init();
}


void MetabAnalysis::runEpoch() {
  som->train(norm_subset->data);
  som->endEpoch();
  //    std::cerr << '.' << std::flush;
  //    std::cerr << "Epoch " << i << std::endl;
}

void MetabAnalysis::endRun() {
  //  std::vector<int> bestmatches;
  for(unsigned int i=0; i < subset->inputs(); i++) {
    esom::Vector ve = norm_subset->data(i);
    bestmatches.push_back((*bestmatch)(ve));
  }

  umatrix = new esom::UMatrix(*distance, *grid);
  umatrix->calculate();
  //  writeUMatrix("umatrix.csv", um);
  esom::Watershed ws(*umatrix);
  tree = ws.tree(bestmatches);
}


void MetabAnalysis::run() {
  beginRun();
  std::cerr << "Training " << std::flush;
#ifdef _WIN32
  double t1 = (double)timeGetTime() * 1e-3;
#else
  struct timeval tv;
  gettimeofday(&tv, 0);
  double t1 = tv.tv_sec + (double)tv.tv_usec * 1e-6;
#endif
  for(unsigned int i=0; i < params.esom_epochs; i++) {
    runEpoch();
  }
  std::cerr << std::endl;
#ifdef _WIN32
  double t2 = (double)timeGetTime() * 1e-3;
#else
  gettimeofday(&tv, 0);
  double t2 = tv.tv_sec + (double)tv.tv_usec * 1e-6;
#endif
  std::cerr << "Training time: " << std::setprecision(4) << (t2 - t1) 
	    << " s" << std::endl;
  endRun();
}
