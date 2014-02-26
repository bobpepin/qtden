#ifndef QTDEN_METAB_CSV
#define QTDEN_METAB_CSV

#include <vector>
#include <fstream>
#include <iostream>

#include <esom.h>

#include "dataset.h"

class CsvReader {

  const char *fname;
  std::ifstream is;
  Dataset& dataset;
  Labels& column_labels;
  Labels& row_labels;
  unsigned int rows, columns, header_rows;

  typedef enum { Header, Skip, Data } column_type;

  void readRow(unsigned int row, const std::vector<column_type>& colspec);
  void readData(const std::vector<column_type>& colspec);
  void readHeader(const std::vector<column_type>& colspec, std::string* = 0);
  void readDimensions();

 public:
  CsvReader(const char *fname, Dataset& dataset)
    : fname(fname), is(fname), dataset(dataset), 
    column_labels(dataset.column_labels), 
    row_labels(dataset.row_labels), header_rows(0) {}
  bool is_open() {
    return is.is_open();
  }
  bool readMetab();
  bool readTable();
  bool readMagic();
};

#endif
