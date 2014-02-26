#include "heatmapplot.h"

#include <sstream>
#include <QMouseEvent>
#include <qwt_plot_canvas.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_color_map.h>
#include <qwt_scale_widget.h>

static void df_collect_ids(const esom::LabelTree& tree, unsigned int node, 
	      std::vector<unsigned int>& out) {
  if(tree.isLeaf(node)) {
    out.push_back(tree.leafValue(node));
    return;
  }
  df_collect_ids(tree, tree.left(node), out);
  df_collect_ids(tree, tree.right(node), out);
}
  

HeatmapRasterData::HeatmapRasterData(const Dataset* dataset, 
				     const esom::LabelTree& tree) 
  : QwtRasterData(), dataset(dataset), tree(tree), order(0), orderEnabled(true) {
  setBoundingRect(QwtDoubleRect(0.0, 0.0, 
				dataset->columns(), 
				dataset->rows()));
  df_collect_ids(tree, tree.top(), order);
  
  std::vector<unsigned int> mark(tree.leafCount());
  for(unsigned int i=0; i < order.size(); i++) {
	  assert(order[i] < mark.size());
	  mark[order[i]] = 1;
  }
  std::vector<unsigned int> unmarked;
  for(unsigned int i=0; i < mark.size(); i++) {
	  if(mark[i] == 0) {
		  unmarked.push_back(i);
	  }
  }
  assert(unmarked.size() == 0);
//  std::cerr << "order[ " << order.size() << "] =";
//   for(unsigned int i=0; i < order.size(); i++) {
//     std::cerr << ' ' << order[i];
//   }
//   std::cerr << std::endl;
}

void HeatmapRasterData::setDataset(const Dataset* ds) {
  dataset = ds;
}

void HeatmapRasterData::toggleOrder(bool flag) {
  orderEnabled = flag;
}

QwtRasterData* HeatmapRasterData::copy() const {
  return new HeatmapRasterData(dataset, tree);
}

QwtDoubleInterval HeatmapRasterData::range() const {
  const esom::Matrix& data = dataset->data;
  double max = data(0, 0);
  double min = max;
  for(unsigned int i=0; i < data.rows(); i++) {
    for(unsigned int j=0; j < data.columns(); j++) {
      double v = data(i, j);
      if(v > max) 
	max = v;
      if(v < min)
	min = v;
    }
  }
  double mm = std::max(std::abs(min), std::abs(max));
  return QwtDoubleInterval(-mm, mm);
}

double HeatmapRasterData::value(double x, double y) const {
  unsigned int iy = (unsigned int)y;
  unsigned int i = iy;
  if(orderEnabled) {
    if(iy >= order.size()) {
      iy = order.size() - 1;
    }
    i = order[iy];
  } else {
    if(i >= dataset->rows()) {
      i = dataset->rows() - 1;
    }
  }
  unsigned int j = (unsigned int)x;
  if(j >= dataset->columns()) 
    j = dataset->columns() - 1;
  // std::cerr << "value: order =";
  // for(unsigned int i=0; i < order.size(); i++) {
  //   std::cerr << ' ' << order[i];
  // }
  // std::cerr << std::endl;
  double v = dataset->data(i, j);
  if(v != v) return 0; // check for NaN
  return v;
}

HeatmapPlot::HeatmapPlot(const MetabData& metab_data, 
			 const esom::LabelTree& tree,
			 const MetabParams& params,
			 QWidget *parent)
  : QwtPlot(parent), metab_data(metab_data), 
    dataset(&metab_data.samples), dataset_tstat(&metab_data.samples_tstat),
    tree(tree), params(params), rasterData(dataset_tstat, tree) { 

  showSamples();
  setMouseTracking(true);
  canvas()->setMouseTracking(true);
  replot();
}

void HeatmapPlot::plotDataset() {
  clear();
  rasterData.setDataset(dataset_tstat);
  rasterData.toggleOrder(order_enabled);

  QwtPlotSpectrogram *spectro = new QwtPlotSpectrogram();

  //  QwtLinearColorMap colorMap(Qt::darkBlue, Qt::white);
  QwtLinearColorMap colorMap(Qt::blue, Qt::red);
  colorMap.addColorStop(0.5, Qt::white);
  // colorMap.addColorStop(0.25, Qt::green);
  spectro->setColorMap(colorMap);

  //  rasterData.setBoundingRect(spectro->boundingRect());
  spectro->setData(rasterData);
  spectro->attach(this);
  //  QBrush sbrush(QColor(255, 255, 255));
  //  QPen spen(QColor(255, 255, 255));
  QwtScaleWidget *rightAxis = axisWidget(QwtPlot::yRight);
  rightAxis->setTitle("Standard deviations from mean");
  rightAxis->setColorBarEnabled(true);
  rightAxis->setColorMap(spectro->data().range(), spectro->colorMap());
  setAxisScale(QwtPlot::yRight, 
	       spectro->data().range().minValue(),
	       spectro->data().range().maxValue() );
  enableAxis(QwtPlot::yRight);
  setAxisScale(QwtPlot::yLeft, 0, dataset->inputs());
  setAxisScale(QwtPlot::xBottom, 0, dataset->dimension());
  replot();
}

void HeatmapPlot::showReplicates() {
  dataset = &metab_data.replicates;
  dataset_tstat = &metab_data.replicates_tstat;
  if(metab_data.samples_dimension == MetabData::Columns) {
    order_enabled = true;
  } else {
    order_enabled = (params.esom_data == MetabParams::Replicates);
  }
  plotDataset();
}

void HeatmapPlot::showSamples() {
  dataset = &metab_data.samples;
  dataset_tstat = &metab_data.samples_tstat;
  if(metab_data.samples_dimension == MetabData::Columns) {
    order_enabled = true;
  } else {
    order_enabled = (params.esom_data == MetabParams::Samples);
  }
  plotDataset();
}

void HeatmapPlot::showSampleStds() {
  dataset = &metab_data.samples_std;
  dataset_tstat = &metab_data.samples_cv;
  if(metab_data.samples_dimension == MetabData::Rows) {
    order_enabled = (params.esom_data == MetabParams::Samples);
  } else {
    order_enabled = true;
  }
  plotDataset();
}

void HeatmapPlot::mouse2data(QMouseEvent *event, 
			     unsigned int& row, unsigned int& col) {
  QPoint canvas_pos = canvas()->pos();
  QPoint pos = event->pos();

  double v = invTransform(QwtPlot::xBottom, pos.x() - canvas_pos.x());
  int j = std::floor(v);
  col = j;
  
  v = invTransform(QwtPlot::yLeft, pos.y() - canvas_pos.y());
  int i = std::floor(v);
  row = i;
}

void HeatmapPlot::mousePressEvent(QMouseEvent *event) {
  unsigned int i, j;
  mouse2data(event, i, j);
  if(i < dataset->inputs() && j < dataset->dimension()) {
    unsigned int row = rasterData.orderEnabled ? rasterData.order[i] : i;
    if(dataset == &metab_data.replicates) {
      emit selectReplicate(row);
    } else {
      emit selectSample(row);
    }
  }
}

void HeatmapPlot::mouseMoveEvent(QMouseEvent *event) {
  unsigned int i, j;
  mouse2data(event, i, j);
  std::stringstream label;

  int leafCount = tree.leafCount();
  int orderSize = rasterData.order.size();
  int datasetRows = dataset->inputs();
  if(rasterData.orderEnabled) {
	  assert(datasetRows == orderSize);
	  assert(leafCount == datasetRows);
  }
  if(i < dataset->inputs() && j < dataset->dimension()) {
    unsigned int row = rasterData.orderEnabled ? rasterData.order[i] : i;
    label << "[ " << dataset->column_labels.name(j) << " ] " 
	  << dataset->row_labels.name(row)
	  << ": " << dataset->data(row, j);
  }
  emit legend(QString(label.str().c_str()));
}
