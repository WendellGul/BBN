//
// Created by Great on 2017/11/24.
//

#include "Node.h"

Node::Node(string name) : name(name),
                          parents(),
                          children(), hidden(false),
                          cpt(), prob(), delta() {
}

Node::Node(string name, bool hidden) : name(name),
                                       parents(),
                                       children(), hidden(hidden),
                                       cpt(), prob(), delta() {

}

/**
 * 随机初始化CPT表
 */
void Node::initCPT() {
    if(!cpt.empty())
        return;

    int num = (int)parents.size();
    if(num == 0) {
        cpt["t"] = 0;
        cpt["f"] = 0;
    }
    else {
        for (int i = 0; i < (int) pow(2, num); i++) {
            vector<int> x;
            int k = i;
            while (k / 2 != 0) {
                x.push_back(k % 2);
                k /= 2;
            }
            x.push_back(k);
            while(x.size() < num)
                x.push_back(0);
            for (int j = 0; j < 2; j++) {
                x.push_back(j);
                string pat = getParentsPattern(vector<int>(), x);
                cpt[pat] = 0;
                x.pop_back();
            }
        }
    }

    set<string> inited;
    for(auto i : cpt) {
        string key1 = i.first;
        if(inited.find(key1) != inited.end())
            continue;
        string key2 = key1;
        int last = (int)key1.size() - 1;
        key2[last] = key1[last] == 't' ? 'f' : 't';
        cpt[key1] = 1.0;
        cpt[key2] = 1.0;
        inited.insert(key1);
        inited.insert(key2);
    }

    // 初始化prob
    for(auto i : cpt) {
        prob[i.first] = 0;
    }
}


const string &Node::getName() const {
    return name;
}

const vector<Node *> &Node::getParents() const {
    return parents;
}

const vector<Node *> Node::getChildren() const {
    return children;
}

void Node::setParents(vector<Node *> parents) {
    this->parents = parents;
    for(Node * p : this->parents) {
        p->addChild(this);
    }
}

void Node::setCpt(const map<string, double> &cpt) {
    this->cpt = cpt;
}

void Node::setCpt(const string key, const double value) {
    cpt[key] = value;
}

void Node::addChild(Node* child) {
    children.push_back(child);
}

double Node::getCondProb(string &cond) {
    return cpt[cond];
}


/**
 * 计算P(x_ij, u_ik | d)并累加
 * @param nodePos
 * @param j 结点取值
 * @param k parents模式
 * @param d 一条数据
 */
void Node::updateP(map<string, int> nodePos, int j, int k, vector<int> d) {
    vector<int> pos, x(d.size(), 0);
    for(Node *p : parents) {
        pos.push_back(nodePos[p->name]);
    }
    if(!pos.empty())
        sort(pos.begin(), pos.end());

    int p = nodePos[name];
    if(parents.empty()) {
        x[p] = j;
        pos.push_back(p);
    }
    else {
        vector<int> values;
        while(k / 2 != 0) {
            values.push_back(k % 2);
            k /= 2;
        }
        values.push_back(k);
        if(values.size() < parents.size())
            values.push_back(0);
        for(int i = 0; i < values.size(); i++)
            x[pos[i]] = values[values.size() - i - 1];

        x[p] = j;
        pos.push_back(p);
    }
    string key = getParentsPattern(pos, x);
    prob[key] += intersect(pos, x, d);
}

/**
 * 根据父结点组合模式k生成位置序列和取值序列
 * @param nodePos
 * @param k 父结点组合模式
 * @return
 */
vector<vector<int>> Node::getPosAndValue(map<string, int> nodePos, int k) {
    vector<vector<int>> result;
    vector<int> pos, value(nodePos.size(), 0);

    if(!parents.empty()) {
        for (Node *p : parents) {
            pos.push_back(nodePos[p->name]);
        }

        sort(pos.begin(), pos.end());

        vector<int> x;
        while (k / 2 != 0) {
            x.push_back(k % 2);
            k /= 2;
        }
        x.push_back(k);
        if (x.size() < parents.size())
            x.push_back(0);
        for (int i = 0; i < x.size(); i++)
            value[pos[i]] = x[x.size() - i - 1];
    }

    result.push_back(pos);
    result.push_back(value);

    return result;
}

/**
 * 根据当前节点取值j和父结点组合模式k生成位置序列和取值序列
 * @param nodePos
 * @param j 当前节点取值
 * @param k 父结点模式
 * @return
 */
vector<vector<int>> Node::getPosAndValue(map<string, int> nodePos, int j, int k) {
    vector<vector<int>> result = getPosAndValue(nodePos, k);
    result[0].push_back(nodePos[name]);
    result[1][nodePos[name]] = j;
    return result;
}

/**
 * 获取类似"ttt"的字符串
 * @param pos pos[:-2]为父结点, pos[-1]为子结点; pos为空时用x序列生成
 * @param x 取值序列
 * @return key of CPT map
 */
string Node::getParentsPattern(vector<int> pos, vector<int> x) {
    string key = "";
    if(pos.empty()) {
        for(int i : x) {
            key = i == 1 ? key.append("t") : key.append("f");
        }
    }
    else {
        for (int i : pos) {
            key = x[i] == 1 ? key.append("t") : key.append("f");
        }
    }
    return key;
}

/**
 * 更新CPT值
 */
void Node::updateW() {
    for(auto i : prob) {
        delta[i.first] = i.second / cpt[i.first];
        cpt[i.first] += delta[i.first];
        prob[i.first] = 0;
    }
}

/**
 * 归一化
 */
void Node::normalizeW() {
    set<string> normalized;
    for(auto i : cpt) {
        string key1 = i.first;
        if(normalized.find(key1) != normalized.end())
            continue;
        string key2 = key1;
        int last = (int)key1.size() - 1;
        key2[last] = key1[last] == 't' ? 'f' : 't';
        double sum = cpt[key1] + cpt[key2];
        cpt[key1] /= sum;
        cpt[key2] /= sum;
        normalized.insert(key1);
        normalized.insert(key2);
    }
}

/**
 * 返回delta的最大值
 */
double Node::getDelta() {
    double re = 0.0;
    for(auto i : delta)
        re = max(re, i.second);
    return re;
}

/**
 * 打印CPT表
 */
void Node::printCPT() {
    cout << "Node: " << name;
    cout << "\tParents: ";
    if(parents.empty())
        cout << "NULL" << endl;
    else {
        for(Node * p : parents) {
            cout << p->name << ", ";
        }
        cout << endl;
    }
    for(auto i : cpt) {
        cout << "P(\"" << i.first << "\") = " << i.second << endl;
    }
    cout << endl;
}

Node::~Node() {
    parents.clear();
    children.clear();

    cpt.clear();
    prob.clear();
    delta.clear();
}

bool Node::isHidden() const {
    return hidden;
}

const map<string, double> &Node::getCpt() const {
    return cpt;
}




