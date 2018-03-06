//
// Created by Great on 2017/11/27.
//

#include "BBN.h"

/**
 * 生成一个贝叶斯信念网络
 * @param name bbn的名字
 * @param nodes 结点集合（要求父结点在子节点之前定义）
 * @param limit Gibbs sampling limit
 */
BBN::BBN(const string &name, vector<Node *> nodes) : name(name), nodes(nodes),
                                                     nodeNames(), nodePos(),
                                                     nodeMap(), e(clock()), u(0, 1) {
    for(int i = 0; i < nodes.size(); i++) {
        Node *n = nodes[i];
        nodeNames.push_back(n->getName());
        nodeMap[n->getName()] = n;
        nodePos[n->getName()] = i;
    }
    initCPT();
}

/**
 * 初始化CPT表
 */
void BBN::initCPT() {
    for(auto i : nodes)
        i->initCPT();
}

/**
 * 从文件中读取CPT表
 * @param cptFile
 */
void BBN::setCPTFromFile(string cptFile) {
    map<string, map<string, double>> cpts = readCPTFile(cptFile);
    for(auto cpt : cpts)
        nodeMap[cpt.first]->setCpt(cpt.second);
}

string BBN::getNodeName(int pos) {
    return nodeNames[pos];
}

/**
 * 学习cpt中的value
 * @param data 训练集
 */
void BBN::train(vector<vector<int>> &rawData) {
    vector<vector<int>> data = preprocessData(rawData, 2);
    if(consistHidden())
        trainPartOb(data);
    else
        trainFullOb(data);
}

/**
 * 预处理数据，为隐变量赋值
 * @param rawData 不含隐变量的数据
 * @param flag 0 表示隐变量赋值为0，1 表示赋值为1，否则随机赋值
 * @return 返回含有隐变量的数据
 */
vector<vector<int>> BBN::preprocessData(vector<vector<int>> &rawData, int flag) {
    vector<vector<int>> data;
    set<int> hiddenPos = getHiddenNodes();
    if(hiddenPos.empty())
        data = rawData;
    else {
        for (auto d : rawData) {
            vector<int> item = d;
            for(int pos : hiddenPos) {
                if(flag <= 1)
                    item.insert(item.begin() + pos, flag);
                else {
                    int value = u(e) > 0.5 ? 1 : 0;
                    item.insert(item.begin() + pos, value);
                }
            }
            data.push_back(item);
        }
    }
    return data;
}

/**
 * 训练数据全观测BBN
 * @param data
 */
void BBN::trainFullOb(vector<vector<int>> &data) {
    for(Node * i : nodes) {
        int end = (int) pow(2, i->getParents().size());
        for(int k = 0; k < end; k++) {
            // 父结点的位置和取值
            vector<vector<int>> pPosAndValue = i->getPosAndValue(nodePos, k);

            // 父结点和当前节点的位置和取值
            vector<vector<vector<int>>> cPosAndValues;

            int pCount = 0;
            vector<int> cCounts;
            for(int j = 0; j < 2; j++) {
                cPosAndValues.push_back(i->getPosAndValue(nodePos, j, k));
                cCounts.push_back(0);
            }

            // 统计取值
            for(vector<int> d :data) {
                pCount += intersect(pPosAndValue[0], pPosAndValue[1], d);
                for(int j = 0; j < cCounts.size(); j++) {
                    cCounts[j] += intersect(cPosAndValues[j][0], cPosAndValues[j][1], d);
                }
            }

            // 更新概率
            for(int j = 0; j < 2; j++) {
                string key = i->getParentsPattern(cPosAndValues[j][0], cPosAndValues[j][1]);
                i->setCpt(key, (cCounts[j] + 1.0) / pCount);
            }
        }
        i->normalizeW();
    }
}

/**
 * 训练数据部分观测BBN
 * @param data
 */
void BBN::trainPartOb(vector<vector<int>> &data) {
    double delta;
    int k = 0;
    do {
        delta = 0.0;
        for (Node *i : nodes) {
            // k的取值为2^parentsNum
            // k表示i的父结点的组合方式的个数
            int end = (int) pow(2, i->getParents().size());
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < end; k++) {
                    for (vector<int> d : data) {
                        // 计算P(x_ij, u_ik | d)
                        i->updateP(nodePos, j, k, d);
                    }
                }
            }
            i->updateW();
            delta = max(i->getDelta(), delta);
        }
        k++;
        if(k % 100 == 0)
            printf("iteration: %d\tdelta: %f\n", k, delta);
    } while(delta > 0.1);

    // 归一化
    for(Node *i : nodes)
        i->normalizeW();
}

/**
 * 计算P(x_q=c_q|x_e)的值
 * @param q
 * @param e
 * @return P(x_q=c_q|x_e)的值
 */
double BBN::inference(map<string, int> &q, map<string, int> &e) {
    int count = 0;
    int pos = nodePos[q.begin()->first];

    // 做limit次Gibbs采样
    for(int i = 0; i < 500; i++) {
        vector<int> sample = gibbsSampling(e);
        if (sample[pos] == q.begin()->second)
            count++;
    }
    return count * 1.0 / 500;
}

/**
 * 预测名为nodeName的结点的取值
 * @param nodeName
 * @param e
 * @return
 */
int BBN::predict(string nodeName, map<string, int> &e) {

    // 初始化sample
    vector<int> sample((int)nodeNames.size(), 0);
    for(auto it : e) {
        // 获取e中各个变量的位置
        int p = nodePos[it.first];
        // 固定e中各个变量的值
        sample[p] = it.second;
    }

    // 计算x_i在其他变量当前取值下的条件概率
    // 即计算P(x_i|MB(x_i))和P(!x_i|MB(x_i))
    // conditions为条件概率的分解，即对x_i|MB(x_i)的分解
    list<vector<Node *>> conditions;
    vector<Node *> nodes;
    Node* n = getNode(nodeName);

    // 该结点的父结点
    if(!n->getParents().empty())
        nodes.insert(nodes.end(), n->getParents().begin(), n->getParents().end());

    // 该结点
    nodes.push_back(n);

    // P(该结点|该结点的父结点)
    conditions.push_back(nodes);

    // 该结点的子结点和子结点的父结点
    for(Node* child : n->getChildren()) {
        nodes.clear();
        nodes.insert(nodes.end(), child->getParents().begin(), child->getParents().end());
        nodes.push_back(child);

        // P(该结点的子结点|子结点的父结点)
        conditions.push_back(nodes);
    }

    // 计算条件概率
    vector<double> prob = calCondProbability(n, conditions, sample);

    return prob[0] > prob[1] ? 0 : 1;

}

/**
 * 给定某些结点e进行Gibbs采样
 * @param e 给定的结点
 * @return 采样序列
 */
vector<int> BBN::gibbsSampling(map<string, int> &e) {
    // 初始化sample
    vector<int> sample((int)nodeNames.size(), 0);
    for(auto it : e) {
        // 获取e中各个变量的位置
        int pos = nodePos[it.first];
        // 固定e中各个变量的值
        sample[pos] = it.second;
    }
    vector<int> newSample(sample);
    while(true) {
        sample = newSample;

//        cout << "第" << k << "次：";
//        for(int i : sample)
//            cout << i;
//        cout << endl;
        for(string name : nodeNames) {

            // 对其他变量进行采样
            if(e.find(name) == e.end()) {
                // 计算x_i在其他变量当前取值下的条件概率
                // 即计算P(x_i|MB(x_i))和P(!x_i|MB(x_i))
                // conditions为条件概率的分解，即对x_i|MB(x_i)的分解
                list<vector<Node *>> conditions;
                vector<Node *> nodes;
                Node* n = getNode(name);

                // 该结点的父结点
                if(!n->getParents().empty())
                    nodes.insert(nodes.end(), n->getParents().begin(), n->getParents().end());

                // 该结点
                nodes.push_back(n);

                // P(该结点|该结点的父结点)
                conditions.push_back(nodes);

                // 该结点的子结点和子结点的父结点
                for(Node* child : n->getChildren()) {
                    nodes.clear();
                    nodes.insert(nodes.end(), child->getParents().begin(), child->getParents().end());
                    nodes.push_back(child);

                    // P(该结点的子结点|子结点的父结点)
                    conditions.push_back(nodes);
                }

                // 计算条件概率
                vector<double> prob = calCondProbability(n, conditions, newSample);

                // 随机采样更新x_i
                int pos = nodePos[name];
                newSample[pos] = randomSample(prob[0]);
            }
        }
        if(BBN::equal(sample, newSample))
            break;
    }
//    cout << "收敛！" << endl;
    return sample;
}

Node* BBN::getNode(string &name) {
    return nodeMap[name];
}

/**
 * 计算已知变量x_i的马尔可夫毯的条件概率P(x_i|MB(x_i))和P(!x_i|MB(x_i))
 * @param n 结点x_i
 * @param conditions 转换后的条件概率，每个list中vector[1:]是vector[0]的父结点
 * @param sample 各个结点x的取值
 * @return P(x_i|MB(x_i))和P(!x_i|MB(x_i))的值
 */
vector<double> BBN::calCondProbability(Node *n, list<vector<Node *>> &conditions, vector<int> &sample) {
    vector<double> prob = {1.0, 1.0};
    for(vector<Node *> nodes : conditions) {
        vector<double> cpv = getCPTValue(n, nodes, sample);
//        cout << "CPT value: " << cpv[0] << " " << cpv[1] << endl;
        for(int i = 0; i < 2; i++)
            prob[i] *= cpv[i];
    }
    double sum = prob[0] + prob[1];
    for(int i = 0; i < 2; i++)
        prob[i] /= sum;
    return prob;
}


/**
 * 根据概率prob进行采样
 * @param prob prob x取值为0的概率
 * @return 采样的结果: 0或1
 */
int BBN::randomSample(double &prob) {
    return u(e) < prob ? 0 : 1;
}

/**
 * 获取CPT表中的节点n取值为0或1的条件概率值
 * @param n
 * @param nodes nodes[0:-2]是nodes[-1]的父结点
 * @param sample 条件节点的取值，初始化为0
 * @return 节点n取值为0或1的条件概率值
 */
vector<double> BBN::getCPTValue(Node *n, vector<Node *> nodes, vector<int> &sample) {
    vector<double> cpv;
    Node* cptNode = nodes[nodes.size() - 1];
    for(int i = 0; i < 2; i++) {
        string cond = "";
        for (Node* node : nodes) {
            int pos = nodePos[node->getName()];
            if(i == 0)
                cond = sample[pos] == 1 ? cond + "t" : cond + "f";
            else {
                if(n->getName() == node->getName())
                    cond += "t";
                else
                    cond = sample[pos] == 1 ? cond + "t" : cond + "f";
            }
        }
        cpv.push_back(cptNode->getCondProb(cond));
    }
    return cpv;
}

bool BBN::equal(vector<int> a, vector<int> b) {
    bool flag = true;
    for(int i = 0; i < a.size(); i++) {
        if(a[i] != b[i]) {
            flag = false;
            break;
        }
    }
    return flag;
}

/**
 * 前向采样
 * @return 一次采样的序列
 */
vector<int> BBN::forwardSampling() {
    set<Node *> curNodes, children, sampledNodes;

    // 先对根结点（没有父结点的结点）采样
    for(Node *n : nodes) {
        if(n->getParents().empty()) {
            curNodes.insert(n);
        }
    }
    vector<int> sample(nodes.size(), 0);

    while(sampledNodes.size() < nodes.size()) {
        for (Node *r : curNodes) {
            // 判断结点r是否已经采样
            if (sampledNodes.find(r) != sampledNodes.end())
                continue;
            string name = r->getName();

            // 获取结点r的条件结点
            vector<Node *> condNodes = r->getParents();
            condNodes.push_back(r);

            // 获取相应CPT表的值
            vector<double> cptValue = getCPTValue(r, condNodes, sample);
            int pos = nodePos[name];

            // 采样
            sample[pos] = randomSample(cptValue[0]);

            // 记录已采样的结点
            sampledNodes.insert(r);

            // 获取所有子结点
            if (!r->getChildren().empty()) {
                for (Node *c : r->getChildren())
                    children.insert(c);
            }
        }
        curNodes = children;
    }

    return sample;
}

/**
 * 生成测试数据
 * @param k 测试数据的记录条数
 */
void BBN::generateData(int k) {
    cout << "Generate " << k << " records:" << endl;
    for(int i = 1; i < k + 1; i++) {
        vector<int> sample = forwardSampling();
        for(int s : sample)
            cout << s;
        cout << endl;
    }
}


/**
 * 生成数据并保存
 * @param k 数据条数
 * @param fileName 文件名
 */
void BBN::generateData(int k, string fileName) {
    cout << "Generate " << k << " incomplete records to file " << fileName << "." << endl;

    set<int> hiddenPos = getHiddenNodes();
    ofstream fout(fileName);
    for(string name : nodeNames) {
        if(hiddenPos.find(nodePos[name]) == hiddenPos.end())
            fout << name << ",";
        else
            fout << "[" << name << "],";
    }
    fout << endl;
    for(int i = 1; i < k + 1; i++) {
        vector<int> sample = forwardSampling();
        for(int j = 0; j < sample.size(); j++) {
            if(hiddenPos.find(j) == hiddenPos.end())
                fout << sample[j];
        }
        fout << endl;
    }
    fout.close();
}

/**
 * 如果存在隐结点，返回隐结点的pos
 */
set<int> BBN::getHiddenNodes() {
    set<int> pos;
    for(Node * n : nodes) {
        if(n->isHidden())
            pos.insert(nodePos[n->getName()]);
    }
    return pos;
}

/**
 * 判断是否包含隐结点
 * @return
 */
bool BBN::consistHidden() {
    for(Node * n : nodes) {
        if(n->isHidden())
            return true;
    }
    return false;
}

/**
 * 打印CPT表
 */
void BBN::printCPT() {
    cout << "==========================================" << endl;
    for(auto i : nodes) {
        i->printCPT();
    }
    cout << "==========================================" << endl;
}

/**
 * 保存CPT表到文件
 * @param fileName
 */
void BBN::saveCPT(string fileName) {
    ofstream fout(fileName);
    for(string name : nodeNames) {
        fout << name << endl;
        for(auto cpt : nodeMap[name]->getCpt()) {
            fout << cpt.first << "," << cpt.second << endl;
        }
        fout << "#" << endl;
    }
    fout.close();
}

BBN::~BBN() {
    for(auto i : nodes)
        delete(i);
    nodes.clear();
    nodeNames.clear();
    nodeMap.clear();
    nodePos.clear();
}





