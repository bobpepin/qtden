#ifndef QTDEN_LRN_H
#define QTDEN_LRN_H

void readLrnData(std::istream& is, esom::Matrix data,
		 std::vector<int> column_types, Labels& row_labels);
void readLrnHeader(std::istream& is, int& inputs, int& dimension, 
		   std::vector<int>& column_types, 
		   Labels& column_labels);
void readLrnMap(std::istream& is, std::map<unsigned int, std::string>&);
void readLrnMap(std::istream& is, std::map<unsigned int, unsigned int>&);

#endif
