//
// Created by Great on 2017/11/29.
//

#include "util.h"

/**
 * 从文件中读取数据
 * @param fileName 文件名
 * @param header 首行
 * @return
 */
vector<vector<int>> readDataFromFile(string fileName, bool header) {
    ifstream fin(fileName);
    if(!fin.good()) {
        cout << "文件不存在！" << endl;
        return vector<vector<int>>();
    }
    vector<vector<int>> data;
    while(!fin.eof()) {
        string values;
        vector<int> item;
        if(header) {
            fin >> values;
            header = false;
            continue;
        }
        fin >> values;

        for(char c : values)
            item.push_back(c - '0');

        if(!item.empty())
            data.push_back(item);
    }
    fin.close();
    return data;
}

/**
 * 从文件中读取CPT表
 * @param fileName
 * @return
 */
map<string, map<string, double>> readCPTFile(string fileName) {
    ifstream fin(fileName);
    if(!fin.good()) {
        cout << "文件不存在！" << endl;
        return map<string, map<string, double>>();
    }
    map<string, map<string, double>> cpts;
    string name;
    map<string, double> cpt;
    while(!fin.eof()) {
        string str;
        fin >> str;
        if(str.empty())
            continue;
        if(str == "#") {
            cpts[name] = cpt;
            cpt.clear();
            continue;
        }
        vector<string> splitStr = split(str, ",");
        if(splitStr.size() == 1) {
            name = splitStr[0];
        }
        else {
            string cond = splitStr[0];
            istringstream iss(splitStr[1]);
            double prob;
            iss >> prob;
            cpt[cond] = prob;
        }
    }
    fin.close();
    return cpts;
}

/**
 * 若两个集合在pos位置上的元素全部一致
 * 则返回1，否则返回0
 */
int intersect(vector<int> pos, vector<int> a, vector<int> b) {
    int re = 1;
    for(int p : pos) {
        if(a[p] != b[p]) {
            re = 0;
            break;
        }
    }
    return re;
}

vector<string> split(const string &s, const string &seperator) {
    vector<string> result;
    typedef string::size_type string_size;
    string_size i = 0;

    while(i != s.size()) {
        //找到字符串中首个不等于分隔符的字母；
        int flag = 0;

        while(i != s.size() && flag == 0) {
            flag = 1;
            for(string_size x = 0; x < seperator.size(); ++x)
                if (s[i] == seperator[x]) {
                    ++i;
                    flag = 0;
                    break;
                }
        }

        //找到又一个分隔符，将两个分隔符之间的字符串取出；
        flag = 0;
        string_size j = i;
        while(j != s.size() && flag == 0) {
            for(string_size x = 0; x < seperator.size(); ++x)
                if(s[j] == seperator[x]) {
                    flag = 1;
                    break;
                }
            if(flag == 0)
                ++j;
        }
        if(i != j) {
            result.push_back(s.substr(i, j - i));
            i = j;
        }
    }
    return result;
}

/**
 * 保存预测结果
 * @param predict 预测结果
 * @param result 实际结果
 * @param fileName
 */
void saveResult(vector<int> predict, vector<int> result, string fileName) {
    ofstream fout(fileName);
    fout << "predict\tresult" << endl;
    for(int i = 0; i < result.size(); i++)
        fout << predict[i] << "\t" << result[i] << endl;
    fout.close();
}

void printVector(vector<int> a) {
    for(int i : a)
        cout << i << " ";
    cout << endl;
}
