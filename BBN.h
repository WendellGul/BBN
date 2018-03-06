//
// Created by Great on 2017/11/27.
//

#ifndef BBN_BBN_H
#define BBN_BBN_H

#include "Node.h"
#include <list>
#include <iostream>
#include <fstream>

/**
 * bayesian belief network
 */
class BBN {
    string name;
    vector<Node*> nodes;  // 结点集合
    vector<string> nodeNames;  // 结点名
    map<string, Node*> nodeMap;  // 结点名对应结点

    default_random_engine e;
    uniform_real_distribution<double> u;

public:
    map<string, int> nodePos;  // 结点对应位置

private:
    // Gibbs采样
    vector<int> gibbsSampling(map<string, int> &e);

    // 前向采样
    vector<int> forwardSampling();

    // 通过名字获取结点
    Node* getNode(string &name);

    vector<double> calCondProbability(Node *node, list<vector<Node *>> &conditions, vector<int> &sample);

    int randomSample(double &prob);

    vector<double> getCPTValue(Node *n, vector<Node*> nodes, vector<int> &sample);

    void initCPT();

    set<int> getHiddenNodes();

    bool consistHidden();

    // 训练不含隐变量的BBN
    void trainFullOb(vector<vector<int>> &data);

    // 训练含有隐变量的BBN
    void trainPartOb(vector<vector<int>> &data);

public:
    BBN(const string &name, vector<Node *> nodes);

    void setCPTFromFile(string cptFile);

    string getNodeName(int pos);

    // 训练bbn，得到cpt表参数
    void train(vector<vector<int>> &rawData);

    vector<vector<int>> preprocessData(vector<vector<int>> &rawData, int flag);

    // 计算概率P(q|e)
    double inference(map<string, int> &q, map<string, int> &e);

    // 预测
    int predict(string nodeName, map<string, int> &e);

    // 生成k条数据并打印
    void generateData(int k);

    // 生成k条数据并存入文件
    void generateData(int k, string fileName);

    void printCPT();

    void saveCPT(string fileName);

    static bool equal(vector<int> a, vector<int> b);

    ~BBN();
};


#endif //BBN_BBN_H
