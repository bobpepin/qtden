#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>
#include <esom.h>

#include "dataset.h"

void skipline(std::istream& is, char term = '\n') {
  char ch;
  while(is.get(ch) && ch != term) { }
}

double norm(esom::Vector v) {
  esom::Vector::iterator i;
  double n2 = 0;
  for(i = v.begin(); i != v.end(); i++) {
    n2 += (*i)*(*i);
  }
  return sqrt(n2);
}

void writeGrid(const char *fname, esom::Grid& grid) {
  std::ofstream os(fname);
  int cols = grid.columns();
  for(unsigned int i=0; i < grid.size(); i++) {
    esom::Vector v = grid.getModel(i);
    os << norm(v) << '\t';
    if((int)grid.index2col(i) % cols == cols-1) {
      os << std::endl;
    }
  }
  os.close();
}

void writeBestmatches(const char *fname, std::vector<int> bestmatches, 
		      esom::Grid& grid, const Dataset& dataset) {
  std::ofstream bm_ofs(fname);
  for(unsigned int i=0; i < bestmatches.size(); i++) {
    bm_ofs << grid.index2row(bestmatches[i]) << "\t";
    bm_ofs << grid.index2col(bestmatches[i]) << "\t";
    bm_ofs << dataset.row_labels.id(i) << std::endl;
  }
  bm_ofs.close();
}

void writeUMatrix(const char *fname, esom::UMatrix& um) {
  std::ofstream umatrix_ofs(fname);
  int cols = um.columns();
  for(unsigned int i=0; i < um.size(); i++) {
    umatrix_ofs << um.getValue(i);
    if((int)um.index2col(i) % cols == cols-1) {
      umatrix_ofs << std::endl;
    } else {
      umatrix_ofs << '\t';
    }
  }
  umatrix_ofs.close();
}

void readLrnHeader(std::istream& is, int& inputs, int& dimension, 
		   std::vector<int>& column_types, 
		   Labels& column_labels) {
  std::vector<std::string> column_names;
  char c;
  is >> c >> inputs >> c >> dimension;
  dimension--;
  is >> c;
  assert(c == '%');
  column_types.resize(0);
  is >> c;
  while(c != '%') {
    column_types.push_back(c - '0');
    is >> c;
  }
  std::string label;
  for(unsigned int i=0; i < column_types.size(); i++) {
    if(column_types[i] != 0) {
      is >> label;
      if(column_types[i] == 1) {
      //      std::cerr << "Label = " << label << std::endl;
	column_names.push_back(label);
      }
    }
  }
  column_labels.setNames(column_names);
}

void readLrnData(std::istream& is, esom::Matrix data,
		 std::vector<int> column_types, Labels& row_labels) {
  unsigned int inputs = data.rows();
  std::vector<unsigned int> row_ids(inputs);
  for(unsigned int i=0; i < inputs; i++) {
    int k = 0;
    for(unsigned int j=0; j < column_types.size(); j++) {
      //      std::cerr << "Column type: " << column_types[j] << " ";
      if(column_types[j] == 0) {
	//	std::cerr << "Label";
	skipline(is, '\t');
	skipline(is, '\t');
      } else if(column_types[j] == 9) {
	is >> row_ids[i];
	//	std::cerr << "Id: " << row_ids[i] << std::endl;
      } else if(column_types[j] == 1) {
	double d;
	is >> d;
	data(i, k) = d;
	k++;
	//	std::cerr << "Value: " << d;
      }
      //            std::cerr << " " << j << std::endl;
    }
  }
  row_labels.setIds(row_ids);
}

void readLrnMap(std::istream& is, std::map<unsigned int, std::string>& names) {
  char c;
  unsigned int rows;
  unsigned int id;
  std::string name;
  is >> c >> rows;
  assert(c == '%' && rows > 0);
  while(!is.eof()) {
    is >> id;
    is.get(c);
    assert(c == '\t');
    getline(is, name);
    names[id] = name;
  }
}

void readLrnMap(std::istream& is, std::map<unsigned int, unsigned int>& groups) {
  char c;
  unsigned int rows;
  unsigned int id, group;
  is >> c >> rows;
  assert(c == '%' && rows > 0);
  while(!is.eof()) {
    is >> id;
    is.get(c);
    assert(c == '\t');
    is >> group;
    skipline(is);
    groups[id] = group;
  }
}

void readLrn(const char *sprefix, Dataset& dataset) {
  int inputs; 
  int dimension;
  std::vector<int> column_types;

  std::string prefix(sprefix);
  std::string lrnFile = prefix + ".lrn";
  std::string namesFile = prefix + ".names";
  std::string groupsFile = prefix + ".cls";
  std::string colGroupsFile = prefix + ".colcls";

  std::ifstream ifs(lrnFile.c_str());
  readLrnHeader(ifs, inputs, dimension, column_types, dataset.column_labels);

  std::cerr << inputs << " inputs, dimension " << dimension << std::endl;
  dataset.resize(inputs, dimension);
  readLrnData(ifs, dataset.data, column_types, dataset.row_labels);
  ifs.close();

  ifs.open(namesFile.c_str());
  if(ifs.is_open()) {
    std::map<unsigned int, std::string> names;
    readNames(ifs, names);
    dataset.row_labels.setIdNames(names);
    ifs.close();
  }
  
  ifs.open(groupsFile.c_str());
  if(ifs.is_open()) {
    std::map<unsigned int, unsigned int> groups;
    readGroups(ifs, groups);
    dataset.row_labels.setGroups(groups);
    ifs.close();
  }


  ifs.open(colGroupsFile.c_str());
  if(ifs.is_open()) {
    std::map<unsigned int, unsigned int> groups;
    readGroups(ifs, groups);
    dataset.column_labels.setGroups(groups);
    ifs.close();
  }
}


std::vector<unsigned int> read_column_order(const char *prefix) {
  std::string fname = std::string(prefix) + ".colcls";
  std::ifstream ifs(fname.c_str());
  std::vector<unsigned int> r;
  char c;
  ifs >> c;
  assert(c == '%');
  skipline(ifs);
  while(!ifs.eof()) {
    unsigned int col;
    ifs >> col;
    r.push_back(col);
    skipline(ifs);
  }
  return r;
}

#if 0
esom::LabelTree processDataset(Dataset& dataset) {
  const int epochs = 20;
  esom::ToroidGrid grid(50, 82, dataset.dimension());
  esom::distance::Euclidean distance;
  esom::bestmatch::Linear bestmatch(distance);
  esom::neighbourhood::Gauss neighbourhood;
  esom::cooling::Linear cooling(.5, .1, epochs);
  esom::cooling::Linear radiusCooling(50, 1, epochs);
  esom::OnlineSOM som(grid, bestmatch, neighbourhood, cooling, radiusCooling);

  srand(time(NULL));
  //  srand(1);
  som.init();

  writeGrid("grid_before.csv", grid);

  //  norm_level_col(dataset);

  for(int i = 0; i < epochs; i++) {
    som.train(dataset);
    som.endEpoch();
    std::cerr << "Epoch " << i << std::endl;
  }

  writeGrid("grid_after.csv", grid);

  std::vector<int> bestmatches;
  for(unsigned int i=0; i < dataset.inputs(); i++) {
    esom::Vector ve = dataset.data(i);
    bestmatches.push_back(bestmatch(ve));
  }

  writeBestmatches("bestmatches.csv", bestmatches, grid, dataset);

  esom::UMatrix um(distance, grid);
  um.calculate();

  writeUMatrix("umatrix.csv", um);

  esom::Watershed ws(um);
  esom::LabelTree tree = ws.tree(bestmatches);

  return tree;
}
#endif

void writeTree(std::ostream& os, esom::LabelTree& tree, int node, std::vector<std::string>& labels) {
  if(tree.isLeaf(node)) {
    int v = tree.leafValue(node);
    os << "bm" << v << " [label=\"" << labels[v] << "\"];" << std::endl;
    return;
  }
  
  os << "n" << node << " [layer=\"\"];" << std::endl;

  int left = tree.left(node);
  int right = tree.right(node);
  os << "n" << node << " -> ";
  if(tree.isLeaf(left)) {
    os << "bm" << tree.leafValue(left);
  } else {
    os << "n" << left;
  }
  os << ";" << std::endl;

  os << "n" << node << " -> ";
  if(tree.isLeaf(right)) {
    os << "bm" << tree.leafValue(right);
  } else {
    os << "n" << right;
  }
  os << ";" << std::endl;

  writeTree(os, tree, left, labels);
  writeTree(os, tree, right, labels);
}
