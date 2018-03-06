//
// Created by Great on 2017/12/9.
//

#include "PartObBBN.h"

/******************************************************
 * 图结构如下（其中TB为隐结点）
 */
// Pollution        Smoker      Coal Miner
//     |              |             |
//     |              |             |
//     |         +----+-----+  +----+
//     |         |          |  |
//     |         |          V  V
//     |         |    +------------------+
//     |         |    | Tuberculosis(TB) |
//     |         |    +------------------+
//     |         |          |  |     |
//     +-------+ | +--------+  |     |
//             | | |           |     |
//             V V V           |     |
//             Cancer          |     |
//              |  |           |     |
//              |  +-----------|--+  |
//              +-----+  +-----+  |  |
//                    |  |        |  |
//                    V  V        V  V
//                    XRay      Dyspnoea
//
//
/**
 * XRay是否为positive作为输出，其余作为输入
/*******************************************************/

/**
 * 用于生成训练集和测试集
 * @return
 */
BBN cancer() {
    Node *pollution = new Node("Pollution"),
            *smoker = new Node("Smoker"),
            *coalMiner = new Node("CoalMiner"),
            *tb = new Node("TB", true),  // 肺结核
            *cancer = new Node("Cancer"),
            *xRay = new Node("XRay"),
            *dyspnoea = new Node("Dyspnoea");

    vector<Node*> parents;

    parents.push_back(smoker);
    parents.push_back(coalMiner);
    tb->setParents(parents);



    parents.clear();
    parents.push_back(pollution);
    parents.push_back(smoker);
    parents.push_back(tb);
    cancer->setParents(parents);

    parents.clear();
    parents.push_back(cancer);
    parents.push_back(tb);
    xRay->setParents(parents);
    dyspnoea->setParents(parents);

    map<string, double> cpt;
    cpt["t"] = 0.2;
    cpt["f"] = 0.8;
    pollution->setCpt(cpt);

    cpt.clear();
    cpt["t"] = 0.3;
    cpt["f"] = 0.7;
    smoker->setCpt(cpt);

    cpt.clear();
    cpt["t"] = 0.01;
    cpt["f"] = 0.99;
    coalMiner->setCpt(cpt);

    cpt.clear();
    cpt["ttt"] = 0.7;
    cpt["ttf"] = 0.3;
    cpt["tft"] = 0.2;
    cpt["tff"] = 0.8;
    cpt["ftt"] = 0.2;
    cpt["ftf"] = 0.8;
    cpt["fft"] = 0.1;
    cpt["fff"] = 0.9;
    tb->setCpt(cpt);

    cpt.clear();
    cpt["tttt"] = 0.8;
    cpt["tttf"] = 0.2;
    cpt["fttt"] = 0.4;
    cpt["fttf"] = 0.6;
    cpt["tftt"] = 0.5;
    cpt["tftf"] = 0.5;
    cpt["ttft"] = 0.4;
    cpt["ttff"] = 0.6;
    cpt["fftt"] = 0.3;
    cpt["fftf"] = 0.7;
    cpt["ftft"] = 0.2;
    cpt["ftff"] = 0.8;
    cpt["tfft"] = 0.1;
    cpt["tfff"] = 0.9;
    cpt["ffft"] = 0.05;
    cpt["ffff"] = 0.95;
    cancer->setCpt(cpt);

    cpt.clear();
    cpt["ttt"] = 0.95;
    cpt["ttf"] = 0.05;
    cpt["tft"] = 0.9;
    cpt["tff"] = 0.1;
    cpt["ftt"] = 0.5;
    cpt["ftf"] = 0.5;
    cpt["fft"] = 0.1;
    cpt["fff"] = 0.9;
    xRay->setCpt(cpt);

    cpt.clear();
    cpt["ttt"] = 0.95;
    cpt["ttf"] = 0.05;
    cpt["ftt"] = 0.8;
    cpt["ftf"] = 0.2;
    cpt["tft"] = 0.7;
    cpt["tff"] = 0.3;
    cpt["fft"] = 0.2;
    cpt["fff"] = 0.8;
    dyspnoea->setCpt(cpt);

    vector<Node*> nodes;
    nodes.push_back(pollution);
    nodes.push_back(smoker);
    nodes.push_back(coalMiner);
    nodes.push_back(tb);
    nodes.push_back(cancer);
    nodes.push_back(xRay);
    nodes.push_back(dyspnoea);

    return BBN("cancer", nodes);
}

/**
 * 生成没有CPT表，需要学习的BBN
 * @return
 */
BBN cancerNoCPT() {
    Node *pollution = new Node("Pollution"),
            *smoker = new Node("Smoker"),
            *coalMiner = new Node("CoalMiner"),
            *tb = new Node("TB", true),  // 肺结核
            *cancer = new Node("Cancer"),
            *xRay = new Node("XRay"),
            *dyspnoea = new Node("Dyspnoea");

    vector<Node*> parents;

    parents.push_back(smoker);
    parents.push_back(coalMiner);
    tb->setParents(parents);

    parents.clear();
    parents.push_back(pollution);
    parents.push_back(smoker);
    parents.push_back(tb);
    cancer->setParents(parents);

    parents.clear();
    parents.push_back(cancer);
    parents.push_back(tb);
    xRay->setParents(parents);
    dyspnoea->setParents(parents);

    vector<Node*> nodes;
    nodes.push_back(pollution);
    nodes.push_back(smoker);
    nodes.push_back(coalMiner);
    nodes.push_back(tb);
    nodes.push_back(cancer);
    nodes.push_back(xRay);
    nodes.push_back(dyspnoea);

    return BBN("cancer", nodes);
}

void learn(string trainFile, string cptFile) {
    BBN cancerBBN = cancerNoCPT();
    // 读取训练数据
    vector<vector<int>> trainData = readDataFromFile(trainFile, true);

    // 学习CPT表
    cancerBBN.train(trainData);

    // 打印CPT表
    cancerBBN.printCPT();

    // 保存CPT表
    cancerBBN.saveCPT(cptFile);
}

void predict(string testFile, string cptFile) {
    BBN cancerBBN = cancerNoCPT();

    // 从文件读取CPT表
    cancerBBN.setCPTFromFile(cptFile);

    // 读取测试数据
    vector<vector<int>> testData = readDataFromFile(testFile, true);

    int resultPos = cancerBBN.nodePos["XRay"];
    vector<int> result, predict;

//    for(auto data : testData) {
//        map<string, int> q0, e;
//        q0["XRay"] = 0;
//        for(int p = 0; p < data.size(); p++) {
//            string name = cancerBBN.getNodeName(p);
//            if(name == "XRay") {
//                result.push_back(data[resultPos]);
//                continue;
//            }
//            e[name] = data[p];
//        }
//        double p0 = cancerBBN.inference(q0, e);
//        if(p0 > 0.5)
//            predict.push_back(0);
//        else
//            predict.push_back(1);
//    }

    for(auto data : testData) {
        map<string, int> e;
        for(int p = 0; p < data.size(); p++) {
            string name = cancerBBN.getNodeName(p);
            if(name == "XRay") {
                result.push_back(data[resultPos]);
                continue;
            }
            e[name] = data[p];
        }
        int pred = cancerBBN.predict("XRay", e);
        predict.push_back(pred);
    }

    // 保存结果
    saveResult(predict, result, "./result");

    // 计算混淆矩阵各个值
    int tp = 0, tn = 0, fp = 0, fn = 0;
    for(int i = 0; i < result.size(); i++) {
        if(predict[i] == 1) {
            if(result[i] == 1)
                tp++;
            else
                fp++;
        }
        else {
            if(result[i] == 1)
                fn++;
            else
                tn++;
        }
    }

    // 计算正确率
    double accuracyRate = (tp + tn) * 1.0 / result.size();
    cout << "Accuracy: " << accuracyRate << endl;

    // 输出混淆矩阵
    cout << "\t\t" << "Actual Value" << endl;
    cout << "\t\t" << "1" << "\t" << "0" << endl;
    cout << "P 1\t\t" << tp << "\t" << fp << endl;
    cout << "D 0\t\t" << fn << "\t" << tn << endl;

}

void generateData(int k, string type) {
    BBN cancerBBN = cancer();
    //cancerBBN.saveCPT("D:/CLionProjects/BBN/cpt0");
    string filePath = "D:/CLionProjects/BBN/" + type + "_data.txt";
    cancerBBN.generateData(k, filePath);
}