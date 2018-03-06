//
// Created by Great on 2017/11/29.
//

#ifndef BBN_UTIL_H
#define BBN_UTIL_H

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

vector<vector<int>> readDataFromFile(string fileName, bool header);
map<string, map<string, double>> readCPTFile(string fileName);
int intersect(vector<int> pos, vector<int> a, vector<int> b);
vector<string> split(const string &s, const string &seperator);
void saveResult(vector<int> predict, vector<int> result, string fileName);
void printVector(vector<int> a);

#endif //BBN_UTIL_H
