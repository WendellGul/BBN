//
// Created by Great on 2017/11/24.
//

#ifndef BBN_NODE_H
#define BBN_NODE_H

#include "util.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <random>
#include <iostream>

using namespace std;

class Node {
    string name;
    vector<Node*> parents;
    vector<Node*> children;
    map<string, double> cpt;

    bool hidden;

    map<string, int> prob;  // 更新时计算P(x_ij, u_ik | d)
    map<string, double> delta;

public:
    explicit Node(string name);

    Node(string name, bool hidden);

    const string &getName() const;

    const vector<Node*> &getParents() const;

    const vector<Node*> getChildren() const;

    void setParents(vector<Node*> parents);

    void addChild(Node* child);

    void setCpt(const map<string, double> &cpt);

    void setCpt(const string key, const double value);

    double getCondProb(string &cond);

    string getParentsPattern(vector<int> pos, vector<int> x);

    void updateP(map<string, int> nodePos, int j, int k, vector<int> d);

    void updateW();

    vector<vector<int>> getPosAndValue(map<string, int> nodePos, int k);

    vector<vector<int>> getPosAndValue(map<string, int> nodePos, int j, int k);

    void normalizeW();

    double getDelta();

    void initCPT();

    void printCPT();

    bool isHidden() const;

    const map<string, double> &getCpt() const;

    ~Node();
};


#endif //BBN_NODE_H
