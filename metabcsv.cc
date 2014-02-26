#include "metabcsv.h"
#include <iostream>
#include <fstream>

#include "lrn.h"

#define COLSEP ','

void skipline(std::istream& is, char term = '\n');

void CsvReader::readDimensions() {
  std::ifstream is(fname);
  char c;
  rows = 1;
  columns = 1;
  while(is.get(c) && c != '\n') {
    if(c == COLSEP) columns++;
  }
  char prev;
  while(is.good() && !is.eof()) {
    prev = c;
    if(is.get(c) && c == '\n') rows++;
  }
  if(c == '\n' && prev == '\n') {
    //    rows--;
  }
  //  std::cerr << "c = [" << (c == '\n') << "] prev = [" << (prev == '\n') << "]" << std::endl;
  //  std::cerr << "Rows: " << rows << " Columns: " << columns << std::endl;
}

void CsvReader::readHeader(const std::vector<column_type>& colspec, std::string *last_header) {
  header_rows = 1;
  unsigned int col = 0;
  unsigned int k = 0;
  std::string header;
  while(!is.eof() && col < columns) {
    getline(is, header, col == columns-1 ? '\n' : COLSEP);
    //    std::cerr << "col " << col << ": " << colspec[col] << " " << header << std::endl;
    if(colspec[col] == Data) {
      column_labels.setName(k, header);
      k++;
      //      std::cerr << "Header = " << header << std::endl;
    }
    col++;
  }
  if(last_header) {
    *last_header = header;
  }
}

void CsvReader::readData(const std::vector<column_type>& colspec) {
  unsigned int data_cols = std::count(colspec.begin(), colspec.end(), Data);
  dataset.resize(rows - header_rows, data_cols);
  for(unsigned int row = 0; row < rows - header_rows; row++) {
    readRow(row, colspec);
  }
}

void CsvReader::readRow(unsigned int row,
			const std::vector<column_type>& colspec) {
  unsigned int col = 0;
  unsigned int k = 0;
  std::string header;
  double val;
  while(!is.eof() && col < columns) {
    if(colspec[col] == Header) {
      getline(is, header, col == columns-1 ? '\n' : COLSEP);
      //      std::cerr << "Row Header " << row << ": [" << header << "]" << std::endl;
      row_labels.setName(row, header);
    } else if(colspec[col] == Data) {
      is >> val;
      dataset.data(row, k) = val;
      skipline(is, col == columns-1 ? '\n' : COLSEP);
      k++;
    } else {
      skipline(is, col == columns-1 ? '\n' : COLSEP);
    }
    col++;
  }
}
  
bool CsvReader::readMetab() {
  if(!is.good()) {
    return false;
  }
  readDimensions();
  if(is.eof()) {
    std::cerr << "EOF\n";
  }
  std::vector<column_type> colspec(columns, Data);
  colspec[0] = Header;
  for(unsigned int i=1; i < 9; i++) {
    colspec[i] = Skip;
  }
  std::string last_header;
  readHeader(colspec, &last_header);
  if(last_header == "Alternative Identifications") {
    std::cerr << "Skipping Alternative Identifications column.\n";
    colspec[columns-1] = Skip;
  }
  readData(colspec);
  //  std::cerr << "Finished data input.\n";
  return is.good();
}

bool CsvReader::readTable() {
  if(!is.good()) {
    return false;
  }
  readDimensions();
  std::vector<column_type> colspec(columns, Data);
  colspec[0] = Header;
  readHeader(colspec);
  readData(colspec);
  return is.good();
}

bool CsvReader::readMagic() {
  const char metabMagic[] = "Metabolite\tCAS\tSource\tScore\tQuantification Ions\t";
  char buf[sizeof(metabMagic)];
  std::ifstream is(fname);
  is.get(buf, sizeof(buf));
  is.close();
  //  std::cerr << "buf = " << buf << std::endl;
  if(std::equal(metabMagic, metabMagic+sizeof(metabMagic)-1, buf)) {
    std::cerr << "CSV format: Metabolite Detector" << std::endl;
    return readMetab();
  } else {
    std::cerr << "CSV format: Table" << std::endl;
    return readTable();
  }
}
  
