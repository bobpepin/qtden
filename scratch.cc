/* Random code snippets */


// The following correlation distances are wrong unless all the grid models 
// are centered

class CenteredCorrelationBestMatch : public esom::BestMatchFunction {
  const esom::Grid *g;
private:
  double dist(const esom::Vector a, const esom::Vector b) {
    double dot = 0;
    double anorm = 0;
    for(unsigned int i=0; i < a.size(); i++) {
      dot += a[i] * b[i];
      anorm += a[i]*a[i];
    }
    dot *= dot;
    dot /= anorm;
    return dot;
  }
public:
  void init(esom::Grid& grid) {
    g = &grid;
  }

  virtual int operator() (const esom::Vector input) {
    unsigned int maxi = 0;
    double max = 0;
    unsigned int m = g->size();
    for(unsigned int i=0; i < m; i++) {
      double v = dist(g->getModel(i), input);
      if(v > max) {
	max = v;
	maxi = i;
      }
    }
    return maxi;
  }
};

class BLAS_BestMatch_Correlation : public esom::BestMatchFunction {
  const esom::Grid* g;
public:
  void init(esom::Grid& grid) {
    std::cerr << "init: size = " << grid.size() << std::endl;
    g = &grid;
  }

  virtual int operator() (esom::Vector input) {
    return 1;
    unsigned int M = g->size();
    //    std::cerr << "M = " << M << std::endl;
    unsigned int N = g->dimension();
    const double* models = g->models;
    double *cov = new double[M];

    cblas_dgemv(CblasRowMajor, CblasNoTrans, M, N,
		1, models, N, input, 1, 0, cov, 1);
    for(unsigned int i=0; i < M; i++) {
      cov[i] = std::pow(cov[i], 2);
      cov[i] /= cblas_ddot(N, models+(i*N), 1, models+(i*N), 1);
      //      std::cerr << "cov[" << i << "] = " << cov[i] << std::endl;
    }
    unsigned int m = cblas_idamax(M, cov, 1);
    //    std::cerr << "m = " << m << std::endl;
    delete[] cov;
    return m;
  }
};

class PseudoCorrelationDistance : public esom::DistanceFunction {
public:
  virtual double operator() (esom::Vector a, esom::Vector b) {
    double dot = 0;
    double anorm = 0;
    for(unsigned int i=0; i < a.size(); i++) {
      dot += a[i] * b[i];
      anorm += a[i]*a[i];
    }
    dot *= dot;
    dot /= anorm;
    return (1 - dot);
  }
};

// Slow, but literal implementation of the Pearson Correlation Coefficient
class FullCorrelationDistance : public esom::DistanceFunction {
public:
  virtual double operator() (esom::Vector a, esom::Vector b) {
    double dot = 0;
    double amean = stat::mean(a);
    double bmean = stat::mean(b);
    //    std::cerr << "amean = " << amean << " bmean = " << bmean << std::endl;
    double anorm = 0;
    double bnorm = 0;
    for(unsigned int i=0; i < a.size(); i++) {
      double ac = a[i]-amean;
      double bc = b[i]-bmean;
      dot += (ac) * (bc);
      anorm += ac*ac;
      bnorm += bc*bc;
    }
    dot *= dot;
    dot /= anorm;
    dot /= bnorm;
    return (1 - dot);
  }
};

// What it says
void benchmark(std::string label, esom::OnlineSOM& som, esom::Matrix data) {
  unsigned int epochs = 30;
  srand(1);
  som.init();

  std::cerr << "Training " << label << ' ' << std::flush;
  struct timeval tv;
  gettimeofday(&tv, 0);
  double t1 = tv.tv_sec + (double)tv.tv_usec * 1e-6;
  for(unsigned int i = 0; i < epochs; i++) {
    som.train(data);
    som.endEpoch();
    std::cerr << '.' << std::flush;
    //    std::cerr << "Epoch " << i << std::endl;
  }
  gettimeofday(&tv, 0);
  double t2 = tv.tv_sec + (double)tv.tv_usec * 1e-6;
  std::cerr << "\nTraining time: " << std::setprecision(4) << (t2 - t1) 
	    << " s" << std::endl;
}

void bla() {
#undef BENCHMARK
#ifdef BENCHMARK  

#if 0
  FullCorrelationDistance corr_distance;
  esom::bestmatch::Linear bm_naive(corr_distance);
  esom::OnlineSOM som_naive(grid, bm_naive,
			    neighbourhood, cooling, radiusCooling);
  benchmark("Linear, naive correlation", som_naive, norm_subset.data);
#endif

  esom::bestmatch::Linear bm_corr(distance);
  esom::OnlineSOM som_corr(grid, bm_corr,
			   neighbourhood, cooling, radiusCooling);
  benchmark("Linear, standard correlation", som_corr, norm_subset.data);

#if 0
  PseudoCorrelationDistance bm_distance;
  esom::bestmatch::Linear bm_pcorr(bm_distance);
  esom::OnlineSOM som_pcorr(grid, bm_pcorr,
			    neighbourhood, cooling, radiusCooling);
  benchmark("Linear, pseudo correlation", som_pcorr, norm_subset.data);
#endif

  CenteredCorrelationBestMatch corr_bestmatch;
  esom::OnlineSOM som_bmcorr(grid, corr_bestmatch,
			     neighbourhood, cooling, radiusCooling);
  benchmark("Correlation Bestmatch", som_bmcorr, norm_subset.data);

  BLAS_BestMatch_Correlation blas_bestmatch;
  esom::OnlineSOM som_blas(grid, blas_bestmatch,
			   neighbourhood, cooling, radiusCooling);
  benchmark("BLAS Bestmatch", som_bmcorr, norm_subset.data);

  exit(0);
#endif

}
